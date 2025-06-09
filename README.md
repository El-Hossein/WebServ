# 🌐 WebServer: Building a Server in C++98

A lightweight HTTP web server implemented in modern C++ (C++98 standard), designed to help understand how web communication works under the hood.

---

## 📌 What is a Server?

A **server** is a **program** (or sometimes a physical machine) that listens for incoming **requests** and **responds with data or services**.

Its role is simple but vital:
- **Wait for requests**
- **Process them**
- **Send back responses**

A **web server** is a special kind of server that uses the **HTTP protocol** to communicate with web browsers.

---

## 🤝 Client-Server Relationship

The communication between a **client** and a **server** happens using **HTTP**:

- **Client** (e.g., browser, app, `curl`):  
  _“Hey server, give me `/index.html`”_

- **Server** (the web server):  
  _“Here’s the content of `/index.html`”_

🔁 In reality, both client and server are just **programs** that talk over a network.

---

## 🆚 Server vs WebServer

| Term        | Meaning |
|-------------|---------|
| **Server**  | A general program (or machine) that responds to requests (e.g., game server, mail server, database server). |
| **WebServer** | A specific type of server that **speaks HTTP** and **serves web content** (HTML, CSS, images, etc.) over the internet. Accepts requests like `GET`, `POST`, and responds with HTTP status, headers, and content. |

---

## 📤 Where Does a Server Get Its Response From?

| Source             | Example             | Triggered by                        |
|--------------------|---------------------|-------------------------------------|
| Filesystem         | `/index.html`       | Static file request                 |
| CGI Script Output  | `/script.php`       | Execution of a dynamic script       |
| Error Response     | `404`, `500`, `403` | File not found, server error, etc. |
| Program Logic      | `301 Redirect`      | Redirects, auto-index, etc.         |

---

## 🔑 Popular Keywords

| **Keyword**  | **Description** |
|--------------|-----------------|
| `Socket`     | Software endpoint to send/receive data across a network. |
| `Port`       | Identifies a specific service (e.g., 80 for HTTP). |
| `Listening`  | The server is waiting for new connections using `listen()`. |
| `Accepting`  | The server accepts a connection using `accept()`. |
| `Request`    | Data sent by a client (e.g., `GET /index.html HTTP/1.1`). |
| `Response`   | Server's reply (e.g., `HTTP/1.1 200 OK`). |

---

## 🛠 Webserver Responsibilities

1. **Listen**  
   Open a port (like 80 or 8080) and wait for HTTP requests.

2. **Parse HTTP**  
   Read and understand requests like `GET /index.html HTTP/1.1`.

3. **Respond in HTTP**  
   Send well-formed HTTP responses with status codes and headers.

4. **Serve Web Content**  
   Provide files like `.html`, `.css`, `.js`, or images.

5. **Run CGI Scripts**  
   Execute programs or scripts (e.g., `.py`, `.php`) and return output.

---

## 📝 Notes

- A **server** is just a role played by a machine.
- Real servers use operating systems like Linux or macOS.
- A **web server** is what actually **hosts websites**.

---

> 💬 _"Server : wa7d role li kayl3bo chi computer."_  
> 💡 _"Webserver huwa li kayhosti site web."_

---

## 🚧 Status

This project is a work-in-progress, aimed at learning and exploring system-level programming and the HTTP protocol in C++.


