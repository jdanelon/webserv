#!/bin/bash

# Define some colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

container_name="nginxtestcontainer"

# Stop and remove the container if it's already running
if [ "$(docker ps -a -q -f name=$container_name)" ]; then
    echo -e "$YELLOW Stopping and removing existing Nginx container...$NC"
    docker stop $container_name
    docker rm $container_name
fi

# Check if configuration file is passed as argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_nginx.conf>"
    exit 1
fi

conf_file=$1

docker build --build-arg conf_file=$conf_file -t nginxtest ../.

# Start the container
echo -e "$YELLOW"
echo -e "Starting Nginx container...$NC"
docker run -d --name nginxtestcontainer -p 8080:80 nginxtest

# Give the server some time to start
sleep 5

# URLs of the servers to compare
nginx_url="http://localhost:8080"
other_url="localhost:80"

# Define the testing function
test_request() {
    local request_type=$1

    echo -e $YELLOW
    echo "Sending ${request_type} requests..."
    echo -e $NC

    nginx_response=$(curl -s -X $request_type $nginx_url)
    other_response=$(curl -s -X $request_type $other_url)

    echo "$nginx_response" > /tmp/nginx_response.txt
    echo "$other_response" > /tmp/other_response.txt

    if cmp -s /tmp/nginx_response.txt /tmp/other_response.txt; then
        echo -e $YELLOW
        echo "${request_type} responses are the same for both servers."
        echo -e $NC
		echo ""
    else
        echo -e $RED
        echo "${request_type} responses are different. Here are the differences:"
        echo -e $NC
        diff /tmp/nginx_response.txt /tmp/other_response.txt
		echo ""
    fi
}

# Call the testing function for different types of requests
test_request "GET"
test_request "POST"
test_request "DELETE"

# Stop the container
echo "Stopping Nginx container..."
docker stop $container_name

# Clean up the temporary files
rm /tmp/nginx_response.txt /tmp/other_response.txt
