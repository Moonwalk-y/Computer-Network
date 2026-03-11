# Computer-Network
My computer network class's learning notes.My notes and code about network protocol.

---
#### https-server
使用C语言实现最简单的HTTP/HTTPS服务器。
实现的服务器要满足以下要求：

- 同时支持HTTP（80端口）和HTTPS（443端口）：使用两个线程分别监听各自端口。
- 只需支持GET方法，解析客户端的请求报文，并返回相应应答内容（见下表）。
需支持的状态码	        需实现的功能
200 OK	              对于443端口接收的请求，如果程序所在文件夹存在所请求的文件，返回该状态码，以及所请求的文件
301 Moved Permanently	对于80端口接收的请求，返回该状态码，并在应答中使用Location字段表达相应的https URL
206 Partial Content	  对于443端口接收的请求，如果所请求的为部分内容（请求中有Range字段），返回该状态码，以及相应的部分内容
404 Not Found	        对于443端口接收的请求，如果程序所在文件夹没有所请求的文件，返回该状态码
