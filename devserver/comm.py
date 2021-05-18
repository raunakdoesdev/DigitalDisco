import sqlite3
import datetime

con = sqlite3.connect('/var/jail/home/team00/final/queue.db')

room_attr_dict = {
    'room': 0,
    'song': 1,
    'paused': 2,
    'position': 3,
    'time': 4,
}

user_attr_dict = {
    'user': 0,
    'room': 1,
    'pause_changed': 2,
    'song_changed': 3
}

default_room = [None, 'Never Gonna Give You Up', 1, 0, datetime.datetime.now()]
default_user = [None, 'shared', 0, 0]


def create_room_table(cur):
    cur.execute('''CREATE TABLE IF NOT EXISTS rooms
    (room text, song text, paused integer,  position text, timing timestamp)''')


def get_room_attr(room, attr, c=None):
    cur = con.cursor() if c is None else c
    create_room_table(cur)
    room_state = cur.execute('''SELECT * FROM rooms WHERE room == ? ORDER BY timing ASC;''', (room,)).fetchall()
    default_room[0] = room
    if len(room_state) == 0: return None
    room_state = tuple(default_room) if len(room_state) == 0 else room_state[0]
    return room_state[room_attr_dict[attr]]


def set_room_attr(room, attr, val, c=None):
    cur = con.cursor() if c is None else c
    create_room_table(cur)
    room_states = cur.execute('''SELECT * FROM rooms WHERE room == ?;''', (room,)).fetchall()
    default_room[0] = room
    room_states = default_room if len(room_states) == 0 else [list(r) for r in room_states]

    cur.execute('''DELETE FROM rooms WHERE room = ?''', (room,))

    for room_state in room_states:
        room_state[room_attr_dict[attr]] = val
        cur.execute('''INSERT INTO rooms VALUES (?, ?, ?, ?, ?)''', tuple(room_state))

    if c is None: con.commit()


def pop_song(room, c=None):
    cur = con.cursor() if c is None else c
    create_room_table(cur)
    cur.execute('''DELETE FROM rooms WHERE room == ? ORDER BY timing ASC LIMIT 1;''', (room,))
    if c is None: con.commit()


def add_to_queue(room, song, c=None):
    cur = con.cursor() if c is None else c
    create_room_table(cur)
    default_room[0] = room
    default_room[1] = song
    default_room[-1] = datetime.datetime.now()
    cur.execute('''INSERT INTO rooms VALUES (?, ?, ?, ?, ?)''', tuple(default_room))
    if c is None: con.commit()


def reset_queue(room, c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''DELETE FROM rooms WHERE room == ?;''', (room,))
    if c is None: con.commit()


def create_user_table(cur):
    cur.execute('''CREATE TABLE IF NOT EXISTS users (user text, room text,  pause_changed int, song_changed int)''')


def set_user_room_attr(room, attr, val, c=None):
    cur = con.cursor() if c is None else c
    create_user_table(cur)
    users = cur.execute('''SELECT * FROM users WHERE room == ?;''', (room,)).fetchall()  # get users
    cur.execute('''DELETE FROM users WHERE room = ?''', (room,))  # delete them

    for user in users:  # add them back in with mopdification
        user = list(user)
        user[user_attr_dict[attr]] = val
        cur.execute('''INSERT INTO users VALUES (?, ?, ?, ?)''', tuple(user))

    if c is None: con.commit()


def get_user_attr(user, attr, c=None):
    cur = con.cursor() if c is None else c
    create_user_table(cur)
    user_state = cur.execute('''SELECT * FROM users WHERE user == ?;''', (user,)).fetchall()
    user_state = default_user if len(user_state) == 0 else list(user_state[0])
    return user_state[user_attr_dict[attr]]


def set_user_attr(user, attr, val, c=None):
    cur = con.cursor() if c is None else c
    create_user_table(cur)
    user_state = cur.execute('''SELECT * FROM users WHERE user == ?;''', (user,)).fetchall()
    default_user[0] = user
    user_state = default_user if len(user_state) == 0 else list(user_state[0])
    cur.execute('''DELETE FROM users WHERE user = ?''', (user,))
    user_state[user_attr_dict[attr]] = val
    cur.execute('''INSERT INTO users VALUES (?, ?, ?, ?)''', tuple(user_state))
    if c is None: con.commit()


def update_user_room(user, room, c=None):
    cur = con.cursor() if c is None else c
    cur.execute('''DELETE FROM users WHERE user = ?''', (user,))  # delete copies of user
    cur.execute('''INSERT INTO users VALUES (?, ?, ?, ?)''', (user, room, 1, 1))
    con.commit()


def request_handler(request):
    try:
        with open('/var/jail/home/team00/final/requests.txt', 'a+') as f:
            f.write(str(request) + '\n')

        if request['method'] == 'POST':
            message = request['form']['message'].split('|')

            if message[0] == 'user':
                user, room = message[1:]
                update_user_room(user, room)

            if message[0] == 'room':
                room, song, led_mode,times_and_freqs = message[1:]
                add_to_queue(room, song + '*' + led_mode + '*' + times_and_freqs)

            if message[0] == 'reset':
                room = message[1]
                reset_queue(room)

            return 'Success'

        else:
            # App stuff

            if request['values']['reason'] == 'debug':
                cur = con.cursor()
                return str(cur.execute('''SELECT * FROM rooms;''').fetchall()) + '\n' + 'hello' + str(
                    cur.execute('''SELECT * FROM users;''').fetchall())

            if request['values']['reason'] == 'jsquery':
                return get_room_attr(request['values']['room'], 'song').split('*')[0]

            if request['values']['reason'] == 'pause':
                set_room_attr(request['values']['room'], 'paused', 1)
                set_user_room_attr(request['values']['room'], 'pause_changed', 1)

            if request['values']['reason'] == 'play':
                set_room_attr(request['values']['room'], 'paused', 0)
                set_user_room_attr(request['values']['room'], 'pause_changed', 1)
                set_room_attr(request['values']['room'], 'position', request['values']['position'])

            if request['values']['reason'] == 'end':
                reference = datetime.datetime.now() - datetime.timedelta(seconds=2)

                with open('/var/jail/home/team00/final/pop_memory.txt', 'r') as f:
                    time = datetime.datetime.strptime(f.read(), '%Y-%m-%d %H:%M:%S.%f')

                if time < reference:
                    pop_song(request['values']['room'])  # remove top song
                    set_user_room_attr(request['values']['room'], 'song_changed', 1)
                    with open('/var/jail/home/team00/final/pop_memory.txt', 'w') as f:
                        f.write(str(datetime.datetime.now()))

            # Providing data to the esp
            if request['values']['reason'] == 'espquery':
                try:
                    user = request['values']['user']
                    room = get_user_attr(user, 'room')

                    if get_user_attr(user, 'song_changed') == 1:  # song changed
                        set_user_attr(user, 'song_changed', 0)
                        song, led_mode,times_and_freqs = get_room_attr(room, 'song').split('*')

                        # filename = f'/var/jail/home/team00/final/' + song.replace(" ", "") + '.txt'
                        # with open(filename, 'r') as f:
                        return '3,' + str(led_mode) + ',' + times_and_freqs#f.read().strip(' ')

                    if get_user_attr(user, 'pause_changed') == 1:
                        set_user_attr(user, 'pause_changed', 0)
                        return '1' if get_room_attr(room, 'paused') == 1 else '2,' + get_room_attr(room, 'position')
                except:
                    pass
                return '0'  # default nothing happened response
    finally:
        con.close()
