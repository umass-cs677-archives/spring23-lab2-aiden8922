from http.server import HTTPServer, BaseHTTPRequestHandler
from socketserver import ThreadingMixIn
# Import socket module
import socket            
import json
import threading
# Define ip and ports
catalog_ip="192.168.85.128"
catalog_port=8080
order_ip="192.168.85.128"
order_port=8081


class Handler(BaseHTTPRequestHandler):

    def do_GET(self):
        print(threading.currentThread().getName())
        s = socket.socket()        
        # connect to the catalog server
        s.connect((catalog_ip,catalog_port))
        s.send(("lookup "+self.path[1:]).encode())#the stock name can be obtained from self.path
        # receive data from the server and decoding to get the string.
        reply=s.recv(1024).decode();
        # close the connection
        s.close()
        #the reply string consists of status code of the operation, along with fields of the corresponding stock, seperated by space
        reply=reply.split(" ") 
        status_code=(int)(reply[0])
        if status_code > 0:
            reply_json={
                "data": {
                    "name": reply[1],
                    "price": reply[2],
                    "quantity": reply[4],
                    "current trade volume": reply[3],
                    "trade volume limit": reply[5]
                }
            }
        else:
            err_msg={-100:"invaild command",-1:"stock not exists"}
            reply_json={
                "error": {
                    "code": status_code,
                    "message": err_msg[status_code]
                }
            }
        self.send_response(200)
        self.end_headers()
        self.wfile.write(str(reply_json).encode())
    def do_POST(self):
        print(threading.currentThread().getName())
        content_len = int(self.headers.get('Content-Length'))
        post_body = self.rfile.read(content_len)#get the body of the request
        json_arg=json.loads(post_body)  #convert json string to python dictionary

        s = socket.socket()        
        # connect to the catalog server
        s.connect((order_ip,order_port))
        quantity=json_arg["quantity"] 
        if json_arg["type"] == "sell":
            quantity=-quantity
        s.send(("trade "+json_arg["name"]+" "+str(quantity)).encode())
        # receive data from the server and decoding to get the string.
        reply=int(s.recv(1024).decode())
        # close the connection
        s.close()
        #the reply for trade operation is a single status code 
        if reply >= 0:#transaction number(an unsigned number start from 0) will be returned if the action is successful, 
            reply_json={
                "data": {
                    "transaction_number": reply
                }
            }
        else:
            err_msg={-200:"cant connect to the catalog server",-100:"invaild command",-3:"not enough stock remaining", -2:"trade volume reaches upper bound",-1:"stock not exists"}
            reply_json={
                "error": {
                    "code": reply,
                    "message": err_msg[reply]
                }
            }
        self.send_response(200)
        self.end_headers()
        self.wfile.write(str(reply_json).encode())
        
class ThreadingSimpleServer(ThreadingMixIn, HTTPServer):
    pass

def run():
    server = ThreadingSimpleServer(('0.0.0.0', 8082), Handler)
    server.serve_forever()


if __name__ == '__main__':
    run();