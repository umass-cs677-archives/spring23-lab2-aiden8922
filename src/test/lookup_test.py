import requests
import time
  
# api-endpoint
URL = "http://192.168.85.128:8082/AidenCo"
  

# sending get request and saving the response as response object
i=1
sum=0
while(1):
    time1=time.time()
    r = requests.get(url = URL)
    time2=time.time()
    # extracting data in json format
    data = r.text
    sum=sum+time2-time1    
    # printing the output
    print(data,sum/i)
    i=i+1