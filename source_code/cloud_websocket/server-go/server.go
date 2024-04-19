package main

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"github.com/golang-jwt/jwt/v5"
	"github.com/gorilla/websocket"
	"github.com/joho/godotenv"
	"io"
	"log"
	"net/http"
	"net/url"
	"os"
	"sync"
	"time"
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
var channelCommand = make(chan []byte)

// var channelStatus = make(chan []byte)
var upService string

func notFoundHandler(w http.ResponseWriter, r *http.Request) {
	http.Error(w, "404 page not found", http.StatusNotFound)
}

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
	go statusChecker()
	go commandSender(channelCommand)

	log.Println("Starting server on :8080...")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}

func statusChecker() {
	domainList := map[string]string{"local": "https://indicator-he-apparently-universal.trycloudflare.com", "openai": "https://api.openai.com"}
	for {
		for _, domain := range domainList {
			go func(domains string) {
				log.Println("Checking domain:", domains)
				client := &http.Client{
					Timeout: 3 * time.Second, // Set timeout to 10 seconds
				}
				// Make a GET request with the custom client
				ctx, cancel := context.WithTimeout(context.Background(), client.Timeout)

				defer cancel()
				req, err := http.NewRequestWithContext(ctx, "GET", domains, nil)
				if err != nil {
					fmt.Println("Error creating request:", err)
					return
				}
				resp, err := client.Do(req)
				if err != nil {
					if ctx.Err() == context.DeadlineExceeded {
						log.Printf("Timeout when checking domain %s", domains)
					}
					return
				}

				if resp.StatusCode != 421 {
					log.Printf("Domain %s is down. Status code: %d", domains, resp.StatusCode)
				} else {
					log.Printf("Domain %s is up. Status code: %d", domains, resp.StatusCode)
					for key, val := range domainList {
						if val == domain {
							if key != upService && upService != "local" {
								log.Println("Changing service, new service is:", key)
							}
							server.mu.Lock()
							upService = key
							server.mu.Unlock()
							continue
						}
					}

				}
			}(domain)
			time.Sleep(7 * time.Second)
		}
	}
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
	go postCommand(command, channelCommand)
	w.Write([]byte("Processing Request!"))
	fmt.Println("Received command:", command)
}
func postCommand(command string, ch chan []byte) {
	service := upService
	fmt.Println("Service:", service)
	var headers map[string]string
	var url string
	switch string(service) {
	case "openai":
		headers = map[string]string{"Authorization": "Bearer " + os.Getenv("OPENAI_APIKEY"), "Content-Type": "application/json"}
		url = "https://api.openai.com/v1/chat/completions"
	default:
		headers = map[string]string{}
		url = "https://indicator-he-apparently-universal.trycloudflare.com/v1/chat/completions"
	}
	var body interface{} = map[string]interface{}{
		"model": func(service string) string {
			switch service {
			case "local":
				return "koboldcpp/Noromaid-v0.4-Mixtral-Instruct-8x7b.q3_k_m"
			case "openai":
				return "gpt-3.5-turbo"
			default: // default is local
				return "gpt-3.5-turbo"
			}
		}(service),
		"messages": []map[string]interface{}{
			{
				"role":    "system",
				"content": "You are an assistant tasked with controlling two IoT devices: a fan and an RGB LED strip. You will perform tasks related to these devices and respond only to questions related to the system. For example, if the user asks for the status of a device, you will respond accordingly. If the input is not recognizable, respond with an error object: { response: 'error' }.  Fan Device Schema: Power Control: If asked to turn the fan on or off, respond with: { response: 'ok', topic: 'fan/control', payload_format: 'INT', payload: '{0 for OFF or 1 for ON}' }.  Speed Control: If asked to set the fan speed, respond with a number between 96 and 1024 for the duty cycle: { response: 'ok', topic: 'fan/control', payload_format: 'INT', payload: 'number between 96 and 1024' }.  Function Control: If asked to set the Fan to 'breeze' mode, then response with this object : {response: 'ok', topic: 'fan/control', payload_format: 'JSON', payload: {function: 1}} LED Strip: Power Control: If asked to turn the LEDs on or off, respond with: { response: 'ok', topic: 'led/control/power', payload_format: 'INT', payload: '{0 for off, 1 for on}' }.  Color Control: If asked to change the color of the LEDs, respond with the RGB values of the color: { response: 'ok', topic: 'led/control/color', payload_format: 'JSON', payload: { red: {R value}, green: {G value}, blue: {B value} } }.  Function Control: If asked to set the strip to 'rainbow' mode, then response with this object : {response: 'ok', topic: 'fan/control', payload_format: 'JSON', payload: 1}",
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
	log.Println("doing request")
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
	if resp.StatusCode != 200 {
		log.Println("Request failed, falling back to OpenAI...")
		go postCommand(command, ch)
		return
	}
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
