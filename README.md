# Streaming_Audio

The goal of this project is to program a streaming audio platform like Spotify. <br/>
There is a server which have the music and there is a client which ask to play a music. The server will send the data of the music, the client will receive the data and play it in real time. <br/>
The exchange is with the protocol UDP. We can only have one client at a time, if a second client try to connect to the server, he will receive an error message. <br/>
The client can ask to add some options for this music like accelerate, slow down or the volume.

# How to use

You can add musics in the folder "music", so the server can send the data to a client. <br/>
Launch the audioserver with "./audioserver". <br/>
Then launch the client with the command "padsp ./audioclient 127.0.0.1 musicName". <br/>
You have to write padsp before, without it you can't read them music.
If you want to know all the music the server have or the options instead of write the music, you can write help and you will see everything. <br/>
To add the option juste write the music's name and your option like "padsp ./audioclient 127.0.0.1 zelda.wav volume +50".

