import streamlit as st
import streamlit.components.v1 as components


def send_request(message):
    import requests
    url = 'http://608dev-2.net/sandbox/sc/team00/final/comm.py'
    return str(requests.post(url, data={'message': message}).content)[2:-3]


prepend = '../'

songs = {
    'Never Gonna Give You Up': 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Never%20Gonna%20Give%20You%20Up.mp3',
    'Piano Man': 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Piano%20Man.mp3'
}
st.title('Digital Disco')
st.markdown('### App by Team 0')

username = st.text_input(label='User Name')
room = st.selectbox(label='Room', options=['shared', 'room1'])
song = send_request(f'user|{username}|{room}')

st.write('### Media Player:')
with open('webapp/media.html', 'r') as f:
    audio = components.html(f.read().replace('SONGPATHHERE', song).replace('MOTION_NAME_HERE', room),
                            height=50)

song_choice = st.selectbox(label='Song', options=list(songs.keys()))
if st.button('Submit Song'):
    send_request(f'room|{room}|{songs[song_choice]}')
    st.write('Subimtted!')
