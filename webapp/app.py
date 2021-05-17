import streamlit as st
import streamlit.components.v1 as components


def send_request(message):
    import requests
    url = 'http://608dev-2.net/sandbox/sc/team00/final/comm.py'
    return str(requests.post(url, data={'message': message}).content)[2:-3]


prepend = '../'

songs = {
    'Never Gonna Give You Up': 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Never%20Gonna%20Give%20You%20Up.mp3',
    'Piano Man': 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Piano%20Man.mp3',
    'Stayin Alive': 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Stayin%20Alive.mp3',
}
st.title('Digital Disco')
st.markdown('### App by Team 0')

username = st.text_input(label='User Name')
room = st.selectbox(label='Room', options=['shared', 'room1'])
send_request(f'user|{username}|{room}')

st.write('### Media Player:')
with open('webapp/media.html', 'r') as f:
    audio = components.html(f.read().replace('MOTION_NAME_HERE', room)
                            .replace('ROOMGOESHERE', room),
                            height=50)

song_choice = st.selectbox(label='Song', options=list(songs.keys()))
led_mode_dict = {
    'Fast Waves': 0,
    'Slow Waves': 1,
    'Sun Gradients': 2,
    'Moon Gradients': 3,
    'Sparkles': 4,
    'Pulses': 5
}

led_mode = st.selectbox(label='LED Mode', options=list(led_mode_dict.keys()))
if st.button('Add to Queue'):
    send_request(f'room|{room}|{song_choice}|{led_mode_dict[led_mode]}')
    st.write('Submitted!')
