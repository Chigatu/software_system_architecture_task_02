#!/bin/bash

BASE_URL="http://localhost:8080"
echo "========================================"
echo "Medical Records API Testing"
echo "========================================"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print test result
print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ PASSED${NC}: $2"
    else
        echo -e "${RED}✗ FAILED${NC}: $2"
    fi
}

# 1. Health check
echo -e "\n${YELLOW}1. Health Check${NC}"
response=$(curl -s -w "\n%{http_code}" $BASE_URL/health)
http_code=$(echo "$response" | tail -n1)
body=$(echo "$response" | head -n-1)
if [ "$http_code" = "200" ]; then
    print_result 0 "Health check passed"
    echo "   Response: $body"
else
    print_result 1 "Health check failed (HTTP $http_code)"
fi

# 2. Register new user with unique login
TIMESTAMP=$(date +%s)
echo -e "\n${YELLOW}2. Register User${NC}"
response=$(curl -s -w "\n%{http_code}" -X POST $BASE_URL/api/auth/register \
  -H "Content-Type: application/json" \
  -d "{
    \"login\": \"testdoctor_${TIMESTAMP}\",
    \"email\": \"test_${TIMESTAMP}@hospital.ru\",
    \"firstName\": \"Тест\",
    \"lastName\": \"Докторов\",
    \"patronymic\": \"Тестович\",
    \"password\": \"test123\"
  }")
http_code=$(echo "$response" | tail -n1)
body=$(echo "$response" | head -n-1)
if [ "$http_code" = "201" ]; then
    print_result 0 "User registration passed"
    echo "   Response: $body"
else
    print_result 1 "User registration failed (HTTP $http_code)"
    echo "   Response: $body"
fi

# 3. Login to get token
echo -e "\n${YELLOW}3. Login${NC}"
response=$(curl -s -X POST $BASE_URL/api/auth/login \
  -H "Content-Type: application/json" \
  -d "{\"login\":\"testdoctor_${TIMESTAMP}\",\"password\":\"test123\"}")

# Extract token properly using python3 (more reliable)
TOKEN=$(echo "$response" | python3 -c "import sys, json; print(json.load(sys.stdin)['token'])" 2>/dev/null)

if [ -n "$TOKEN" ] && [ "$TOKEN" != "null" ]; then
    print_result 0 "Login successful"
    echo "   Token: ${TOKEN:0:20}..."
    echo "   Full token: $TOKEN"
else
    print_result 1 "Login failed"
    echo "   Response: $response"
    exit 1
fi

# 4. Search users by name mask
echo -e "\n${YELLOW}4. Search Users by Mask${NC}"
response=$(curl -s -w "\n%{http_code}" "$BASE_URL/api/users/search?mask=Доктор")
http_code=$(echo "$response" | tail -n1)
body=$(echo "$response" | head -n-1)
if [ "$http_code" = "200" ]; then
    print_result 0 "User search passed"
    echo "   Response: $body"
else
    print_result 1 "User search failed (HTTP $http_code)"
fi

# 5. Create patient (with auth)
echo -e "\n${YELLOW}5. Create Patient (with auth)${NC}"
response=$(curl -s -w "\n%{http_code}" -X POST $BASE_URL/api/patients \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "firstName": "Анна",
    "lastName": "Смирнова",
    "patronymic": "Ивановна",
    "phone": "+79991112233",
    "address": "г. Москва, ул. Цветочная, д. 10",
    "birthDate": "1985-03-15",
    "snils": "111-222-333 44",
    "policyNumber": "1112223334445556"
  }')
http_code=$(echo "$response" | tail -n1)
body=$(echo "$response" | head -n-1)
if [ "$http_code" = "201" ]; then
    print_result 0 "Patient creation passed"
    echo "   Response: $body"
    PATIENT_ID=$(echo "$body" | python3 -c "import sys, json; print(json.load(sys.stdin)['id'])" 2>/dev/null)
    echo "   Patient ID: $PATIENT_ID"
else
    print_result 1 "Patient creation failed (HTTP $http_code)"
    echo "   Response: $body"
fi

# 6. Create patient without auth (should fail)
echo -e "\n${YELLOW}6. Create Patient (without auth - should fail)${NC}"
response=$(curl -s -w "\n%{http_code}" -X POST $BASE_URL/api/patients \
  -H "Content-Type: application/json" \
  -d '{
    "firstName": "Неавторизованный",
    "lastName": "Пользователь",
    "phone": "+79990001122",
    "address": "г. Москва",
    "birthDate": "1990-01-01",
    "snils": "999-888-777 66",
    "policyNumber": "9998887776665554"
  }')
http_code=$(echo "$response" | tail -n1)
if [ "$http_code" = "401" ]; then
    print_result 0 "Auth protection works (got 401)"
else
    print_result 1 "Auth protection failed (got $http_code)"
fi

# 7. Search patients by name
echo -e "\n${YELLOW}7. Search Patients by Name${NC}"
response=$(curl -s -w "\n%{http_code}" "$BASE_URL/api/patients/search?fullName=Смирнова")
http_code=$(echo "$response" | tail -n1)
body=$(echo "$response" | head -n-1)
if [ "$http_code" = "200" ]; then
    print_result 0 "Patient search passed"
    echo "   Response: $body"
else
    print_result 1 "Patient search failed (HTTP $http_code)"
fi

# 8. Create medical record (with auth)
if [ -n "$PATIENT_ID" ]; then
    echo -e "\n${YELLOW}8. Create Medical Record (with auth)${NC}"
    response=$(curl -s -w "\n%{http_code}" -X POST $BASE_URL/api/medical-records \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $TOKEN" \
      -d "{
        \"patientId\": $PATIENT_ID,
        \"doctorId\": 1,
        \"diagnosisCode\": \"J06.9\",
        \"diagnosisDescription\": \"Острая респираторная инфекция верхних дыхательных путей\",
        \"complaints\": \"Кашель, насморк, температура 38.5, головная боль\"
      }")
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    if [ "$http_code" = "201" ]; then
        print_result 0 "Medical record creation passed"
        echo "   Response: $body"
        RECORD_CODE=$(echo "$body" | python3 -c "import sys, json; print(json.load(sys.stdin)['code'])" 2>/dev/null)
        echo "   Record code: $RECORD_CODE"
    else
        print_result 1 "Medical record creation failed (HTTP $http_code)"
        echo "   Response: $body"
    fi
fi

# 9. Get patient records
if [ -n "$PATIENT_ID" ]; then
    echo -e "\n${YELLOW}9. Get Patient Records${NC}"
    response=$(curl -s -w "\n%{http_code}" "$BASE_URL/api/medical-records/patient/$PATIENT_ID" \
      -H "Authorization: Bearer $TOKEN")
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    if [ "$http_code" = "200" ]; then
        print_result 0 "Get patient records passed"
        echo "   Response: $body"
    else
        print_result 1 "Get patient records failed (HTTP $http_code)"
        echo "   Response: $body"
    fi
fi

# 10. Get record by code
if [ -n "$RECORD_CODE" ]; then
    echo -e "\n${YELLOW}10. Get Record by Code${NC}"
    response=$(curl -s -w "\n%{http_code}" "$BASE_URL/api/medical-records/$RECORD_CODE" \
      -H "Authorization: Bearer $TOKEN")
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    if [ "$http_code" = "200" ]; then
        print_result 0 "Get record by code passed"
        echo "   Response: $body"
    else
        print_result 1 "Get record by code failed (HTTP $http_code)"
    fi
fi

# 11. Test invalid token
echo -e "\n${YELLOW}11. Test Invalid Token${NC}"
response=$(curl -s -w "\n%{http_code}" -X POST $BASE_URL/api/patients \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer invalid_token_123" \
  -d '{
    "firstName": "Test",
    "lastName": "Invalid",
    "phone": "+79990001122",
    "address": "г. Москва",
    "birthDate": "1990-01-01",
    "snils": "111-111-111 11",
    "policyNumber": "1111111111111111"
  }')
http_code=$(echo "$response" | tail -n1)
if [ "$http_code" = "401" ]; then
    print_result 0 "Invalid token rejected (got 401)"
else
    print_result 1 "Invalid token not rejected (got $http_code)"
fi

echo -e "\n========================================"
echo "Testing Complete!"
echo "========================================"
