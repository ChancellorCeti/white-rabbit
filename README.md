# White Rabbit

White Rabbit is an elementary email program built in C. Compilation will provide one executable for a server and another for a client. A variety of commands can be run with the client executable to communicate with other people connecting to the same server.

The project name is inspired by the ["follow the white rabbit" scene from The Matrix](https://www.youtube.com/watch?v=iDIwQTTzstk). Email was cooler when it was done through the terminal, and The Matrix would've been a much more boring movie if Neo was using Gmail. 

This project is built on the following principles that I believe will eventually make it better than Gmail:

- Fast server response time
- No bloatware in the client application. Easy to use, no unnecessary features, no cluttered GUI, and not taking all eternity to open the inbox. 
- Security (work in progress) and data privacy (in contrast to Gmail)
- Freedom to write emails however you want. Markdown, html, any file can be sent as an email. Users should not be confined to writing emails using a nausea-inducing GUI that only lets you send certain types of content.

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
