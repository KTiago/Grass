from pwn import *

context.clear(arch = 'amd64')

server = process('bin/server')
client = process(['bin/client', '127.0.0.1', '1337'])

login = "login Acidburn\n"
passw = "pass CrashOverride\n"

client.sendline("login H0D0R")
log.info("server after login:\n %s" % server.recv(timeout=0.5))

# Clean up
client.close()
server.close()
