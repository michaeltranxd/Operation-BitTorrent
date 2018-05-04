# Operation-BitTorrent

#### CS 296 Project Spring 2018

Hello, this is Operation BitTorrent. This project is aimed to explore P2P(peer to peer) file sharing for learning purposes. We will be using the language C to implement this and try to add more features after getting a working copy of P2P file sharing.

## Features
- Downloads from multiple peers
- Utilizes a tracker as central information
- Can only download files that other peers have
- Able to keep track of peers that are within the network

## Future Goals
- [ ] Implement timeouts
- [ ] Error checking
- [ ] Combine tracker into peer so that we can implement a distributed election service
- [ ] Keeping a list of available files on certain peers
- [ ] Respond to requester if file is unavailable (no peer has it)
- [ ] Use EPOLL to better handle our server/client
- [ ] Add cleanup (signals like SIGINT)


### Description

There are three types: PEER, TRACKER, REQUESTER. NOTE that the REQUESTER and PEER are the SAME program. Look below to see the descriptions of each type.

# TRACKER
The job of the TRACKER is to keep a list of all existing connections so it knows who is in the network and also update that list. This will be a running server that only serves as a middleman between the peers and the requester.

# PEER

The job of the PEER side is to respond to the TRACKER for file availability and sending files to REQUESTER. This is automatically handled in code so the user does not need to worry about it. 

# REQUESTER
The job of the REQUESTER is to ask files (commanded by the user) on the network. The user is prompted by the program to type in the following:

```
hostname port filename
```

Then after extracting the information from the user, it is sent to the TRACKER for further instructions.


# How does it work?
There are two components to the program: peer and tracker. The tracker serves as a no faulty server which has the list of connections. Here are the following scenarios that will help you understand more about how it works.

Assuming that the tracker is online and no peers are connected. The following steps will occur:
1. Start a peer on a desired port. The peer will then prompt the user to type in the hostname port filename (hostname and port should be of the tracker).
2. After typing it in, the peer will connect to the tracker and ask for the file. The tracker will then ask the peers in the server (which is 0, it will not ask the requester because there is no point in asking a file you already have). Since we have not implemented response if file is unavailable, the peer will not know if the file cannot be found.

Assuming that the tracker is online and three peers are connect. The following steps will occur:
1. Start a peer on a desired port. The peer will then prompt the user to type in the hostname port filename (hostname and port should be of the tracker).
2. After typing it in, the peer will connect to the tracker and ask for the file. The tracker will then ask the peers in the server (which is 3). The tracker should receive responses from the peers regarding the file (if they have it or not).
3. If there was no peer that has the file, the list will not be compiled and we will drop the request (END). If there was AT LEAST 1 peer that has the file, we will compile a list and send it back to the requester (CONTINUE).
4. Now the requester receives the list from the requester. Now the requester will schedule the file based on the list. This will depend on the filesize and how many peers have the file. It will divide the work up based on size of pages and then ask the peers for the file. 
5. The requester will receive the parts of the file (does not have to be sequential) and then combine the file.


# Concepts we used
- Networking
- Threading
- Mmap
- Concurrency
- Synchronization
