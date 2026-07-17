@0x9eac6eaebbaf4807;
# Top-level message Type sent/received by the server/client
struct UserMessageType {
	command :union {
		logon @0	:UserLogon;
		setNick @1	:UserSetNick;
		sendChat @2 :UserSendChat;
	}
}

# Sent from client to server when a user is logging in
struct UserLogon {
	name @0 :Text;
}

# Sent from client to server when a user is updating their nickname
struct UserSetNick {
	newName @0 :Text;
}

# Sent from client to server when a user is sending a chat message
# Sent from server to client when broadcasting a chat message to other users
struct UserSendChat {
	contents @0 :Text;
	timestamp @1 :Int32;
}