import streamlit as st
import streamlit.components.v1 as components

import os


def send_request(message):
    import requests
    url = 'http://608dev-2.net/sandbox/sc/team00/final/comm.py'
    x = requests.post(url, data={'message': message})


prepend = '../'

songs = {
    'Never Gonna Give You Up': 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Never%20Gonna%20Give%20You%20Up.mp3',
    'Piano Man': 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Piano%20Man.mp3'
}
st.title('Digital Disco')
st.markdown('### App by Team 0')

song = st.selectbox(label='Song', options=list(songs.keys()))
color = st.text_input(label='Color')

if st.button('Submit'):
    send_request(color + song)

    st.write('### Media Player:')
    with open('webapp/media.html', 'r') as f:
        components.html(f.read().replace('SONGPATHHERE', songs[song]), height=8000)
