from var.jail.home.team00.final.note_onset import beats

def request_handler(request):
    if request['method'] == 'POST':
        with open('/var/jail/home/team00/final/temp.txt', 'w') as f:
            f.write(request['form']['message'] + '\n')
        return 'Success'

    else:
        with open('/var/jail/home/team00/final/temp.txt', 'r') as f:
            songname = '/var/jail/home/team00/final/' + f.read().split(None, 1)[1] + '.wav'
            times = beats(songname)
            #format: len(times), times[0], times[1], ... times[n-1], (note: NO trailing comma)
            n = len(times)
            out = str(n) + ','
            for t in times:
                out += '{:.4f}'.format(t) + ','
            return out[:-1]
