from http.server import HTTPServer, BaseHTTPRequestHandler
from socketserver import ThreadingMixIn
# Import socket module
import socket            
import json

# Define ip and ports
catalog_ip="192.168.85.128"
catalog_port=8080
order_ip="192.168.85.128"
order_port=8081


class Handler(BaseHTTPRequestHandler):

    def do_GET(self):
        s = socket.socket()        
        # connect to the catalog server
        s.connect((catalog_ip,catalog_port))
        #print(b"lookup "+self.path[1:])
        s.send(("lookup "+self.path[1:]).encode())
        # receive data from the server and decoding to get the string.
        reply=s.recv(1024).decode();
        # close the connection
        s.close()
        reply=reply.split(" ") 
        print(reply)
        if reply[0] > '0':
            reply_json={
                "data": {
                    "name": reply[1],
                    "price": reply[2],
                    "quantity": reply[4],
                }
            }
        else:
            reply_json={
                "error": {
                    "code": reply[0],
                    "message": "stock not found"
                }
            }
        self.send_response(200)
        self.end_headers()
        self.wfile.write(str(reply_json).encode())
    def do_POST(self):
        
        content_len = int(self.headers.get('Content-Length'))
        post_body = self.rfile.read(content_len)
        print(post_body)
        json_arg=json.loads(post_body)  #convert json string to python dictionary

        s = socket.socket()        
        # connect to the catalog server
        s.connect((order_ip,order_port))
        #print(b"lookup "+self.path[1:])
        quantity=json_arg["quantity"] 
        if json_arg["type"] == "sell":
            quantity=-quantity
        s.send(("trade "+json_arg["name"]+" "+str(quantity)).encode())
        print(("trade "+json_arg["name"]+" "+str(quantity)).encode())
        # receive data from the server and decoding to get the string.
        reply=int(s.recv(1024).decode())
        # close the connection
        s.close()

        if reply > 0:
            reply_json={
                "data": {
                    "transaction_number": reply
                }
            }
        else:
            reply_json={
                "error": {
                    "code": reply,
                    "message": "stock not found"
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