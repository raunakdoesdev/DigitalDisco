import sqlite3
import datetime

con = sqlite3.connect('/var/jail/home/team00/final/main.db')


def update_room_song(room, song, c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''CREATE TABLE IF NOT EXISTS rooms (room text, song text)''')
    cur.execute('''DELETE FROM rooms WHERE room = ?''', (room,))  # delete copies of user
    cur.execute('''INSERT INTO rooms VALUES (?, ?)''', (room, song))
    con.commit()
    if c is not None: con.close()


def get_room_song(room, c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''CREATE TABLE IF NOT EXISTS rooms (room text, song text)''')
    ret = cur.execute('''SELECT * FROM rooms WHERE room == ?;''', (room,)).fetchall()

    if len(ret) == 0:
        song = 'https://github.com/sauhaardac/DigitalDisco/raw/main/songs/Never%20Gonna%20Give%20You%20Up.mp3'
        update_room_song(room, song, cur)
    else:
        song = ret[0][1]

    if c is not None: con.close()
    return song

def get_room_song_recent(room, c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''CREATE TABLE IF NOT EXISTS rooms (room text, song text, timing timestamp)''')
    recently = datetime.datetime.now()- datetime.timedelta(seconds = 60) # if song has been inserted since last ping time
    ret = cur.execute('''SELECT * FROM rooms WHERE room == ? AND WHERE timing > ?;''', (room,recently)).fetchall()
    if len(ret) == 0:
        return 0
    else:
        song = ret[0][1]

    if c is not None: con.close()
    return song

def update_user_room(user, room, c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''CREATE TABLE IF NOT EXISTS users (user text, room text)''')
    cur.execute('''DELETE FROM users WHERE user = ?''', (user,))  # delete copies of user
    cur.execute('''INSERT INTO users VALUES (?, ?)''', (user, room))
    con.commit()
    if c is not None: con.close()

def get_user_room(user,c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''CREATE TABLE IF NOT EXISTS users (user text, room text)''')
    ret = cur.execute('''SELECT * FROM rooms WHERE user == ?;''', (user,)).fetchall()
    if len(ret) == 0:
        return None
    else:
        room = ret[0][1]
    if c is not None: con.close()
    return room


def request_handler(request):
    if request['method'] == 'POST':
        message = request['form']['message'].split('|')

        if message[0] == 'user':
            user, room = message[1:]
            update_user_room(user, room)
            return get_room_song(room)

        if message[0] == 'room':
            room, song = message[1:]
            update_room_song(room, song)
            return song

        with open('/var/jail/home/team00/final/temp.txt', 'w') as f:
            f.write(request['form']['message'] + '\n')

        return 'Success'

    else:
        if request['values']['reason'] == 'jsquery':
            return get_room_song(request['values']['room'])
        if request['values']['reason'] == 'espquery':
            room = get_user_room(request['values']['user'])
            if room:
                s = get_room_song_recent(room)
                if s:
                    songname = '/var/jail/home/team00/final/' + s.split(None, 1)[1].replace(" ", "").replace("\n", "") + '.txt'
                    with open(songname, 'r') as f:
                        return f.read()
                else:
                    return '0'
            return '0'
