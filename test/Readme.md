# Setup the servers to compare in the test-servers.sh

nginx_url="http://localhost:8080"
other_url="http://google.com"

# Build the container

docker build -t nginxtest .

# Run the tests

bash test-servers.sh

# Other useful commands

docker stop $(docker ps -q) 
docker build -t nginxtest . 
docker run -d -p 8080:80 nginxtest