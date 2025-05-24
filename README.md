# White Rabbit

White Rabbit is an elementary email program built in C. Compilation will provide one executable for a server and another for a client. A variety of commands can be run with the client executable to communicate with other people connecting to the same server.

To use this project on your own machine, simply run the following commands

```bash
git clone https://github.com/ChancellorCeti/white-rabbit.git
cd white-rabbit
make
```

Then in one terminal window, run
```bash
./server.out
```
and in another, run
```bash
./client.out create_user username password
./client.out create_user test_recipient password2
./client.out send_email username password test_recipient subject_line name-of-file-you-wish-to-send
```
This will give a basic demonstration of the features present so far in this project.
