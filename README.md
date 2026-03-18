# Computer-Network
My computer network class's learning notes.My notes and code about network protocol.

---
## https-server 实验一
使用C语言实现最简单的HTTP/HTTPS服务器。
实现的服务器要满足以下要求：

- 同时支持HTTP（80端口）和HTTPS（443端口）：使用两个线程分别监听各自端口。
- 只需支持GET方法，解析客户端的请求报文，并返回相应应答内容（见下表）。

| 状态码 | 状态描述 | 触发条件与功能说明 |
| :--- | :--- | :--- |
| **200** | **OK** | 对于 **443 端口**接收的请求，若请求文件存在，则返回该状态码及文件完整内容。 |
| **301** | **Moved Permanently** | 对于 **80 端口**接收的请求，返回该状态码，并在 `Location` 字段中给出对应的 HTTPS URL。 |
| **206** | **Partial Content** | 对于 **443 端口**接收的请求，若包含 `Range` 字段，则返回该状态码及对应的部分文件内容。 |
| **404** | **Not Found** | 对于 **443 端口**接收的请求，若请求的文件不存在，则返回该状态码。 |
