package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"net/url"
	"os"
	"sync"
	"time"

	"github.com/golang-jwt/jwt/v5"
	"github.com/gorilla/websocket"
	"github.com/joho/godotenv"
)

type Server struct {
	conns map[*websocket.Conn]*ConnectionInfo
	mu    sync.Mutex
}

type ConnectionInfo struct {
	Agent string
}

func NewServer() *Server {
	return &Server{
		conns: make(map[*websocket.Conn]*ConnectionInfo),
	}
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

func verifyToken(tokenString string, secretKey []byte) error {
	token, err := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
		return secretKey, nil
	})

	if err != nil {
		return err
	}
	if !token.Valid {
		return fmt.Errorf("invalid token")
	}
	// fmt.Print(token)
	return nil
}

var server = NewServer()



func notFoundHandler(w http.ResponseWriter, r *http.Request) {
	http.Error(w, "404 page not found", http.StatusNotFound)
}

var ch = make(chan []byte)

func main() {
	
	err := godotenv.Load("../../.env")
	if err != nil {
		log.Fatal("Error loading .env file")

	}

	http.HandleFunc("/ws", wsHandler)
	http.HandleFunc("/commands", commandHandler)
	http.Handle("/8-queen/", http.StripPrefix("/8-queen/", http.FileServer(http.Dir("./8queens"))))

	// Set a custom 404 handler for all unmatched routes
	http.HandleFunc("/", notFoundHandler)

	go commandSender(ch)
	
	log.Println("Starting server on :8080...")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}

func commandHandler(w http.ResponseWriter, r *http.Request) {
	var params map[string]string
	err := json.NewDecoder(r.Body).Decode(&params)
	if err != nil {
		fmt.Println("Error decoding JSON:", err)
		return
	}
	command, ok := params["command"]
	if !ok {
		fmt.Println("command key not found in json request")
		return
	}
	// ch := make(chan []byte)
	go openAIReq("https://api.openai.com/v1/chat/completions", command, ch)
	w.Write([]byte("Processing Request!"))
	fmt.Println("Received command:", command)
}

func wsHandler(w http.ResponseWriter, r *http.Request) {
	tokenString := r.URL.Query().Get("token")
	if tokenString == "" {
		fmt.Println("Token not provided")
		http.Error(w, "Token not provided", http.StatusBadRequest)
		return
	}

	// Verify the token
	if err := verifyToken(tokenString, []byte(os.Getenv("JWT_SECRET"))); err != nil {
		fmt.Println("Token verification failed:", err)
		http.Error(w, "Token verification failed", http.StatusUnauthorized)
		return
	}
	agentString := r.URL.Query().Get("agent")
	if agentString == "" {
		fmt.Println("Agent not provided")
		http.Error(w, "Agent not provided", http.StatusBadRequest)
		return
	}

	ws, err := upgrader.Upgrade(w, r, nil)
	if err != nil {

		log.Println(err)
		return
	}

	fmt.Println("New incoming connection from Client:", r.RemoteAddr)
	fmt.Printf("URL: %v\n", r.URL)
	server.mu.Lock()
	server.conns[ws] = &ConnectionInfo{
		Agent: agentString}
	server.mu.Unlock()
	defer func() {
		delete(server.conns, ws)
		ws.Close()
	}()

	for {
		messageType, message, err := ws.ReadMessage()
		if err != nil {
			log.Println(err)
			return
		}
		fmt.Println("Received message:", string(message))
	
		broadcast(messageType, message, ws)
	}
}

// This code talks about the broadcast function of the websocket server
func broadcast(messageType int, b []byte, authWS *websocket.Conn) {
	if server.conns[authWS].Agent == "client" {
		for ws := range server.conns {
			if server.conns[ws].Agent == "broker" {
				go func(ws *websocket.Conn) {
					if err := ws.WriteMessage(messageType, b); err != nil {
						fmt.Println("Write Error: ", err)
					}
				}(ws)
			}
		}
	}
	if server.conns[authWS].Agent == "broker" {
		for ws := range server.conns {
			if server.conns[ws].Agent == "client" {
				go func(ws *websocket.Conn) {
					if err := ws.WriteMessage(messageType, b); err != nil {
						fmt.Println("Write Error: ", err)
					}
				}(ws)
			}
		}
	}
}

func commandSender(ch chan []byte) {
	fmt.Println("Starting command sender")
	time.Sleep(3 * time.Second)
	var addr = flag.String("addr", "localhost:8080", "http service address")
	key := []byte(os.Getenv("JWT_SECRET"))
	t := jwt.New(jwt.SigningMethodHS256)
	s, err := t.SignedString(key)
	if err != nil {
		return
	}
	u := url.URL{Scheme: "ws", Host: *addr, Path: "/ws", RawQuery: "token=" + s + "&agent=client"}
	log.Printf("connecting to %s", u.String())

	c, _, err := websocket.DefaultDialer.Dial(u.String(), nil)
	if err != nil {
		log.Fatal("dial:", err)
		return
	}
	defer c.Close()
	for {
		select {
		case payload, ok := <-ch:
			if !ok {
				fmt.Print("Channel closed")
				return
			}
			c.WriteMessage(websocket.TextMessage, payload)
		default:
			continue
		}
	}
}

func openAIReq(url string, command string, ch chan []byte) {

	var headers = map[string]string{
		"Authorization": "Bearer " + os.Getenv("OPENAI_APIKEY"),
		"Content-Type":  "application/json",
	}
	var body interface{} = map[string]interface{}{
		"model": "gpt-3.5-turbo-1106",
		"response_format": map[string]interface{}{
			"type": "json_object",
		},
		"messages": []map[string]interface{}{
			{
				"role":    "system",
				"content": "You are an assistant tasked to control two iot devices: a fan, and  RGB LED STRIP.  You will be asked to perform tasks related to the operation of these devices: YOU WILL NOT ANSWER TO ANYTHING ELSE other than if the user asks for question related to the system. for example, what is XXX of YYY device?  if the input is not recognizable respond with an object of: { response: 'error' } FAN DEVICE SCHEMA: power control: if asked to turn fan on/off, you will respond with the following json object: { response: 'ok', topic: 'fan/control', payload_format: 'INT', payload: '{0 for OFF or 1 for ON}' } SPEED CONTROL: if asked to turn fan at a certain speed, have in mind the range for duty cycle is 96 to 1024, (take no request for RPM values) so find an applicable value to satisfy the customer.  response json: { response: 'ok', topic: 'fan/control, payload_format: 'INT', payload: 'number between 96 and 1024' } RGB LED STRIP: POWER CONTROL: if asked to turn the LEDs(strip, lights, whatever applicable name) respond in this format: { response: 'ok', topic: 'led/control/power', payload_format: 'INT', payload: '{0 for off, 1 for on}' } COLOR CONTROL: if asked to change the color, of LEDs(strip, lights, whatever applicable get the rgb values of the color, and respond in this format: { response: 'ok', topic: 'led/control/color', payload_format: 'JSON', payload: { red: {R value from RGB}, green: {G value from RGB}, blue: {B value from RGB} } }}",
			},
			{
				"role":    "user",
				"content": command,
			},
		},
	}
	jsonStr, err := json.Marshal(body)
	if err != nil {
		return
	}
	reader := bytes.NewReader(jsonStr)
	req, err := http.NewRequest("POST", url, reader)
	if err != nil {
		return
	}

	for k, v := range headers {
		req.Header.Add(k, v)
	}

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return
	}
	defer resp.Body.Close()

	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return
	}
	fmt.Printf("response status code: %d\n", resp.StatusCode)
	var response Response
	err = json.Unmarshal(respBody, &response)
	if err != nil {
		return
	}
	fmt.Println(response.Choices[0].Message.Content)
	ch <- []byte(response.Choices[0].Message.Content)
}

type Response struct {
	ID      string `json:"id"`
	Object  string `json:"object"`
	Created int    `json:"created"`
	Model   string `json:"model"`
	Choices []struct {
		Index   int `json:"index"`
		Message struct {
			Role    string `json:"role"`
			Content string `json:"content"`
		} `json:"message"`
		Logprobs     interface{} `json:"logprobs"`
		FinishReason string      `json:"finish_reason"`
	} `json:"choices"`
	Usage struct {
		PromptTokens     int `json:"prompt_tokens"`
		CompletionTokens int `json:"completion_tokens"`
		TotalTokens      int `json:"total_tokens"`
	} `json:"usage"`
	SystemFingerprint string `json:"system_fingerprint"`
}
