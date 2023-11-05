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

test_request_partial() {
  local test_number=$1
  local method=$2
  local url=$3
  local expected_content=$4
  local data=$5
  
  echo "Executing Test #$test_number"

  # Perform the request and store the output
  if [ -z "$data" ]; then
    actual_output=$(curl -s -X "$method" "$url")
  else
    actual_output=$(curl -s -X "$method" "$url" -d "$data")
  fi

  # Check if the expected content is present in the output
  if [[ "$actual_output" == *"$expected_content"* ]]; then
    echo -e "${GREEN}Test #$test_number: $method request to $url passed.${NC}"
  else
    echo -e "${RED}Test #$test_number: $method request to $url failed.${NC}"
    echo -e "Content should have included: $expected_content"
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

test_status_code_post() {
  local test_number=$1
  local method=$2
  local url=$3
  local expected_status_code=$4
  local data=$5

  echo "Executing Status Code Test #$test_number"

  # Perform the request and store only the status code
  actual_status_code=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" "$url" -d "$data")

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
  local expected_file=$5
  local upload_file=$6

  echo "Executing Upload Test #$test_number"

  # Perform the request and store only the status code
  actual_status_code=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" "$url" -F "file=@$upload_file")


  # Compare the uploaded file with the original file and print the status code and the difference between the files
  if cmp -s "$expected_file" "$upload_file"; then
    echo -e "${GREEN}Test #$test_number: $method request to $url passed. Status code matches: $expected_status_code${NC}"
  else
    echo -e "${RED}Test #$test_number: $method request to $url failed. Expected status code: $expected_status_code, Received: $actual_status_code${NC}"
    echo -e "Difference between the files:"
    diff "$expected_file" "$upload_file"
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
 
### 006 No content lenght

BASE_URL="http://localhost:3490/qwer"

test_status_code 6 "POST" "$BASE_URL" 411 ## Created

### 007 - Redirect

BASE_URL="http://localhost:3490/redirect"

test_status_code 7 "GET" "$BASE_URL" 301

### 08 - Redirect location

### 09 - Error page configuration

test_request 9 "GET" "http://localhost:3490/qwerqwr.notfound" "test/test_files/009.txt"

### 10 - Default Error page 

test_request 10 "PUT" "http://localhost:3490/qwerqwr.notfound" "test/test_files/010.txt"

### 11 - Test Upload

test_upload 11 "POST" "http://localhost:3490/upload/" 201 "test/www/upload/store-folder/test16.txt"  "test/upload/test16.txt"

### 12 - Test Delete

# test_status_code 12 "DELETE" "http://localhost:3490/upload/store-folder/test16kb.txt" 200

### 13 - Test Multiple port

test_status_code 13 "GET" "http://localhost:3491/index.html" 200

### 14 - Test Multiple port

test_request 14 "GET" "http://localhost:3491/index.html" "test/test_files/001.txt"

### 15 - Max body size

test_status_code_post 15 "POST" "http://localhost:3491/upload/12341" 413 "name=John&age=20&dname=John&dage=20&hname=John&wage=20&wname=John&hage=20"

### 16 - Forbidden method

test_status_code 16 "GET" "http://localhost:3491/ringo/" 405

### 17 - Forbidden method

test_request 17 "GET" "http://localhost:3491/ringo/" "test/test_files/017.txt"

### 18 - CGI Python GET

test_request_partial 18 "GET" "http://localhost:3492/hello.py?first_name=Ringo&last_name=Star" "Hello Ringo Star"

### 19 - CGI PHP GET

test_request_partial 19 "GET" "http://localhost:3492/calc.php" "Simple Calculator"

### 20 - CGI PHP POST

test_request_partial 20 "POST" "http://localhost:3492/calc.php" "42" "num1=13&num2=29&operator=add"

### 21 - Test Upload

test_upload 21 "POST" "http://localhost:3490/upload/" 201 "test/www/upload/store-folder/test500.txt"  "test/upload/test500.txt"

### 22 - Test Upload zip file

# test_upload 22 "POST" "http://localhost:3490/upload/" 201 "test/www/upload/store-folder/10MB.zip"  "test/upload/10MB.zip"