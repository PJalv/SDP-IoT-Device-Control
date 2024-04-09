# Stage 1: Build the Python component
FROM python:3 as python-builder

# Set the working directory
WORKDIR /app

# Copy the Python requirements file
COPY dashboard_gui/requirements.txt /app/dashboard_gui/

# Copy the Python script
COPY dashboard_gui/ /app/dashboard_gui/

WORKDIR /app/dashboard_gui
RUN python -m venv venv


# RUN bash -c "deactivate"

# Stage 2: Build the Node.js component
FROM node:18 as node-builder

# Set the working directory
WORKDIR /app

# Copy the Node.js files
COPY cloud_websocket/ /app/cloud_websocket/

# Install Node.js dependencies
WORKDIR /app/cloud_websocket/broker-js
RUN npm install

# Stage 3: Create the final image
FROM python:3

# Set the working directory
WORKDIR /app

# Copy the built Python files
COPY --from=python-builder /app/dashboard_gui /app/dashboard_gui

# Copy the built Node.js files
COPY --from=node-builder /app/cloud_websocket /app/cloud_websocket
COPY start.sh /app/start.sh
RUN chmod +x /app/start.sh
COPY prep.sh /app/prep.sh
RUN chmod +x /app/prep.sh
RUN bash -c "bash /app/prep.sh"
COPY .env .

# Expose any necessary ports
EXPOSE 8000

# Install Node.js in the final image
RUN apt-get update && apt-get install -y nodejs

# Command to start both components

CMD ["/bin/bash", "/app/start.sh"]
#  CMD ["sh", "-c", "cd dashboard_gui && ls -la && cd ../cloud_websocket/broker-js && ls -la && pip install eel"]
