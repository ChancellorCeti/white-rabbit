# White Rabbit

White Rabbit is an elementary email program built in C. Compilation will provide one executable for a server and another for a client. A variety of commands can be run with the client executable to communicate with other people connecting to the same server.

To use this project on your own machine, simply run the following commands

```bash
git clone https://github.com/ChancellorCeti/white-rabbit.git
cd white-rabbit
make
```

Then in one terminal window, cd into the cloned directory and run
```bash
mkdir db
touch db/main_db.txt && cat "0" > db/main_db.txt
./server.out
```
and in another, cd into the cloned directory and run
```bash
./client.out create_user username password
./client.out create_user test_recipient password2
./client.out send_email username password test_recipient subject_line name-of-file-you-wish-to-send
./client.out get_inbox recipient password2
./client.out get_outbox username password
./client.out dl_email recipient password2 0
cat 0.txt
```
This will give a basic demonstration of the features present so far in this project.
