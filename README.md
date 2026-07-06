```*This project has been created as part of the 42 curriculum by asbouani, wnid-hsa, nrais.*```

# ft_irc — Internet Relay Chat Server

## Description

ft_irc is an IRC server built from scratch in C++ 98.
IRC (Internet Relay Chat) is a protocol that allows multiple users to communicate in real time through channels.
The server handles multiple clients without the use of threading, the server utilizes non-blocking I/O multiplexing through the poll() system call. It manages the entire lifecycle of a chat session, including client authentication (PASS, NICK, USER), private messaging, and full channel management (JOIN, PART, INVITE, TOPIC, KICK, and MODE).

## Instructions

#### Compilation
```bash
make
```
#### Run the Server
```bash
./ircserv <port> <password>
```
#### Connect With nc (testing)
```bash
nc -C 127.0.0.1 6667
PASS password
NICK emma 
USER emma 0 * :Emma
JOIN #room
```
#### Features

- Handle multiple clients simultaneously without blocking
- Non-blocking I/O using a single poll() event loop
- TCP/IP communication (IPv4)
- Client authentication (PASS, NICK, USER)
- Channel creation and management
- Operator and regular user roles
- The following commands are implemented:
  - `PASS` — authenticate with server password
  - `NICK` — set or change nickname
  - `USER` — set username and realname
  - `JOIN` — join or create a channel
  - `PART` — leave a channel
  - `KICK` — remove a user from a channel (operator only)
  - `INVITE` — invite a user to a channel (operator only)
  - `TOPIC` — view or change channel topic (operator only if +t)
  - `MODE` — change channel modes (i, t, k, o, l)
  - `PRIVMSG` — send messages to users or channels
  - `QUIT` — disconnect from the server

## Resources

- https://modern.ircdocs.horse
- https://beej.us/guide/bgnet/html/split-wide/index.html
- https://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf
- https://medium.com/@m-ibrahim.research/demystifying-select-poll-kernel-internals-and-the-c10k-challenge-6f3f5b5cd632

#### AI Usage

AI (Claude) was used to understand IRC protocol, socket programming, poll(), non-blocking I/O. All code was written and reviewed by the team. No production code was copy-pasted from AI.```