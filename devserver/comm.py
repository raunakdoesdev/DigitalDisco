import sqlite3

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


def update_user_room(user, room, c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''CREATE TABLE IF NOT EXISTS users (user text, room text)''')
    cur.execute('''DELETE FROM users WHERE user = ?''', (user,))  # delete copies of user
    cur.execute('''INSERT INTO users VALUES (?, ?)''', (user, room))
    con.commit()
    if c is not None: con.close()


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
        # with open('/var/jail/home/team00/final/temp.txt', 'r') as f:
        #     s = f.read()
        #     if s == '0\n':
        #         return '0'
        #     songname = '/var/jail/home/team00/final/' + s.split(None, 1)[1].replace(" ", "").replace("\n", "") + '.txt'
        #     with open(songname, 'r') as f2:
        #         out = f2.read().replace("\n", "")
        # with open('/var/jail/home/team00/final/temp.txt', 'w') as f:
        #     f.write('0\n')
        # return out
