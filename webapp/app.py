import streamlit as st
import os


def send_request(message):
    import requests
    url = 'http://608dev-2.net/sandbox/sc/raunakc/final/comm.py'
    x = requests.post(url, data={'message': message})


songs = os.listdir('../songs')
songs = {
    song.split('.')[0]: os.path.join('../songs', song)
    for song in songs
}
st.title('Digital Disco')

song = st.selectbox(label='Song', options=list(songs.keys()))
color = st.color_picker(label='Color for Song')

if st.button('Submit'):
    send_request(color + song)
    st.audio(songs[song])
