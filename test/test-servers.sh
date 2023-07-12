#!/bin/bash

# Start the container
echo "Starting Nginx container..."
docker run -d -p 8080:80 nginxtest

# Give the server some time to start
sleep 5

# URLs of the servers to compare
nginx_url="http://localhost:8080"
other_url="http://google.com"

# Define the testing function
test_request() {
    local request_type=$1

    echo "Sending ${request_type} requests..."

    nginx_response=$(curl -s -X $request_type $nginx_url)
    other_response=$(curl -s -X $request_type $other_url)

    echo "$nginx_response" > /tmp/nginx_response.txt
    echo "$other_response" > /tmp/other_response.txt

    if cmp -s /tmp/nginx_response.txt /tmp/other_response.txt; then
        echo "${request_type} responses are the same for both servers."
		echo ""
    else
        echo "${request_type} responses are different. Here are the differences:"
        diff /tmp/nginx_response.txt /tmp/other_response.txt
		echo ""
    fi
}

# Call the testing function for different types of requests
test_request "GET"
test_request "POST"
test_request "PUT"
test_request "DELETE"

# Stop the container
echo "Stopping Nginx container..."
docker stop $(docker ps -q -f ancestor=nginxtest)

# Clean up the temporary files
rm /tmp/nginx_response.txt /tmp/other_response.txt
