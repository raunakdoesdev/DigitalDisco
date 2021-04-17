def request_handler(request):
    if request['method'] == 'POST':
        with open('/var/jail/home/raunakc/final/temp.txt', 'w') as f:
            f.write(request['form']['message'] + '\n')
        return 'Success'

    else:
        with open('/var/jail/home/raunakc/final/temp.txt', 'r') as f:
            return f.read()