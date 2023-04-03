# importing the requests library
import requests
import time
  
# api-endpoint
URL = "http://192.168.85.128:8082/AidenCo"
  

  

# data to be sent to api
data = "{    \"name\": \"AidenCo\",    \"quantity\": 1,    \"type\": \"sell\" }"


i=1
sum=0
while(1):
    time1=time.time()
    r = requests.post(url = URL, data = data)
    time2=time.time()
    # extracting data in json format
 
    sum=sum+time2-time1    
    # printing the output
    print(r.text,sum/i)
    i=i+1
