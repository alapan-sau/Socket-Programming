# File Transfer using Sockets

## How to run

### Server

First navigate to the server directory and run the server.

```sh
$ gcc server.c -o server
$ ./server
```

### Client

Now, once the server starts running, navigate to the client directory and request for the files `file1`, `file2`, `file3` as follows!

```sh
$ gcc client.c -o client
$ ./client file1 file2 file3
```

## Note

- The socket buffer size is assumed to be more than 10,000 bytes. If that is not the case, decrease the value of `BUFFER_LIMIT` in the source code.

- Only the content data is transferred, so the new and old files are not exact replica in terms of permissions, non-content data etc.

- The size of the files should not exceed 2.5 GB.