# tcp client/server cli application
Client/Server application to load big (>100mb) files with data protection.
- the client sends a list of files from his folder.
- the server requests file for loading via port 8400.
- the client sends the size of the file and starts transferring.
- When file transferring for 10% server requests to change the transferring port to (+5) 8405.
- The rest of the file has to be transferred via a new port
- When the file is transferred for 20%  server requests to change the transferred port to (+5) 8410.
- The rest of the file has to be transferred via a new port..... till 100%

  usage:

  ```git -clone https://github.com/webmessia-h/tcpClientServer.git```

  ```make client server```

  ```cd bin```

  ```./server ip port```

  ```./client ip port```

  client : ```~/pathToFolder```

  server : ```~/pathToFolder/filename```

  server : ```~/savePath/filename```
