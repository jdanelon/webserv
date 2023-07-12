# setup the servers to compare in the test-servers.sh

nginx_url="http://localhost:8080"
other_url="http://google.com"

# Build the container

docker build -t nginxtest .

# Run the tests

bash test-servers.sh