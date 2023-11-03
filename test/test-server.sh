#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Function to perform a test
test_request() {
  local test_number=$1
  local method=$2
  local url=$3
  local expected_output_file=$4
  
  # Read the expected output from the file
  expected_output=$(<"$expected_output_file")

  echo "Executing Test #$test_number"

  # Perform the request and store the output
  actual_output=$(curl -s -X "$method" "$url")

  # Compare the actual and expected output
  if [[ "$actual_output" == "$expected_output" ]]; then
    echo -e "${GREEN}Test #$test_number: $method request to $url passed.${NC}"
  else
    echo -e "${RED}Test #$test_number: $method request to $url failed.${NC}"
    echo -e "Expected (from $expected_output_file): $expected_output"
    echo -e "Received: $actual_output"
  fi
}

test_status_code() {
  local test_number=$1
  local method=$2
  local url=$3
  local expected_status_code=$4

  echo "Executing Status Code Test #$test_number"

  # Perform the request and store only the status code
  actual_status_code=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" "$url")

  # Compare the actual and expected status code
  if [ "$actual_status_code" -eq "$expected_status_code" ]; then
    echo -e "${GREEN}Test #$test_number: $method request to $url passed. Status code matches: $expected_status_code${NC}"
  else
    echo -e "${RED}Test #$test_number: $method request to $url failed. Expected status code: $expected_status_code, Received: $actual_status_code${NC}"
  fi
}

test_upload() {
  local test_number=$1
  local method=$2
  local url=$3
  local expected_status_code=$4
  local file=$5

  echo "Executing Upload Test #$test_number"

  # Perform the request and store only the status code
  actual_status_code=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" "$url" -F "file=@$file")

  # Compare the actual and expected status code
  if [ "$actual_status_code" -eq "$expected_status_code" ]; then
    echo -e "${GREEN}Test #$test_number: $method request to $url passed. Status code matches: $expected_status_code${NC}"
  else
    echo -e "${RED}Test #$test_number: $method request to $url failed. Expected status code: $expected_status_code, Received: $actual_status_code${NC}"
  fi
}

### 001

BASE_URL="http://localhost:3490/index.html"
EXPECTED_OUTPUT_FILE_GET="test/test_files/001.txt"

test_request 1 "GET" "$BASE_URL" "$EXPECTED_OUTPUT_FILE_GET"

### 002

BASE_URL="http://localhost:3490/"
EXPECTED_OUTPUT_FILE_GET="test/test_files/001.txt"

test_request 2 "GET" "$BASE_URL" "$EXPECTED_OUTPUT_FILE_GET"

### 003

BASE_URL="http://localhost:3490/index.html"

test_status_code 3 "GET" "$BASE_URL" 200


### 004

BASE_URL="http://localhost:3490/xxxxxxxxxx"

test_status_code 4 "GET" "$BASE_URL" 404

### 005

BASE_URL="http://localhost:3490/qwer"

test_status_code 5 "PUT" "$BASE_URL" 405  ## 405 Method Not Allowed

### 006

BASE_URL="http://localhost:3490/qwer"

test_upload 6 "POST" "$BASE_URL" 200 "test/upload/test1.txt" 


### 007 - Redirect

BASE_URL="http://localhost:3490/redirect"

test_status_code 7 "GET" "$BASE_URL" 301

### 08 - Redirect location

### A request to your server should never hang forever.