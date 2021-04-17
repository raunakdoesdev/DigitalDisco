import streamlit as st


def send_request(message):
    import requests
    url = 'http://608dev-2.net/sandbox/sc/raunakc/final/comm.py'
    x = requests.post(url, data={'message': message})
    st.write(x.text)


st.title('Digital Disco')
message = st.text_input(label='Message')

if st.button('Send to ESP32'):
    send_request(message)
