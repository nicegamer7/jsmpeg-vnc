##Fork Changes:
- Linux support
- Latest JSMpeg
- Shared clipboard
- File uploading via drag drop
- Socket authentication
- Display switching
- Message server

##

#####Linux Support
Only Linux support currently. Most of the Windows code already exists in the repository, I just haven't got round to testing and setting up automated building.

##

#####Socket Authentication:

`-a secret` would require `?password=secret` appended to the URL. If the password does not match the sockets are closed. Use at your own risk! I'm unsure how secure this is.

##

#####Message Server:
I encountered a issue where when a lot of messages were being sent the image quality would reduce. 
To solve this another socket is opened on the next port (port + 1). If external access is needed remember to port forward both ports (8080 and 8081 if using default ports).

Also, all incoming messages are now serviced on a separate thread, this may improving streaming.

##

#####Display Changing:

I've added support to change displays from the client. Append `?display=50` to the URL to switch to X display 50. 

You can spawn another display using `Xvfb :50`. Likely you'll want to spawn a desktop interface, this can be done the same way VNC servers do with the `xstartup` file. Or you can just launch a application using the DISPLAY variable such as `DISPLAY=:50 xterm`.

##

#####Shared Clipboard:

Clipboard is shared when `ctrl + c` or `ctrl + v` is pressed. You'll need to install `xclip` for this to work. `sudo apt-get install xclip`

##

##### File Uploading:

Drag-drop a file onto the player and it will upload it to the servers `/home/Downloads/` directory. While a upload is in progress sending input will not work.

