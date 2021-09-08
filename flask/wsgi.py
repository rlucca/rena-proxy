from flask import Flask, redirect, request, session, jsonify
from os import urandom

app = Flask(__name__, subdomain_matching=True)
SERVER = 'www.rena'
app.config['SERVER_NAME'] = SERVER + ':8123'
app.config['SECRET_KEY'] = urandom(16)

users = {
    # TODO mudar a senha para MD5
    "teste": "exemplo",
    "admin": "exemplo"
}

roles = {
    'admin': 'admin',
}

SESSION_NAME = 'rena_username'

def main_server():
    return 'http://{}'.format(app.config['SERVER_NAME'])

def session_add(u):
    session[SESSION_NAME] = u

def session_get():
    return session.get(SESSION_NAME, None)

def session_del():
    session.pop(SESSION_NAME, None)

@app.route('/login', methods=['GET'])
def login_get():
    print(request.url)
    url = request.args.get('url', main_server())
    if session_get():
        return redirect(url)
    else:
        user = request.args.get('username')
        pwd = request.args.get('password')
        if pwd is not None and users.get(user) == pwd:
            session_add(user)
            return redirect(url)

    return "Not logged", 403

@app.route('/login', methods=['POST'])
def login_post():
    print(request.url)
    url = request.args.get('url', main_server())
    if session_get():
        return redirect(url)
    else:
        user = request.form.get('username')
        pwd = request.form.get('password')
        if pwd is not None and users.get(user) == pwd:
            session_add(user)
            return redirect(url)

    return "Not logged", 403

@app.route('/logout')
def logout():
    username = session_get()
    session_del()
    return "Logoff"

@app.route('/', subdomain='<path:domain>')
@app.route('/<path:e>', subdomain='<path:domain>')
#@app.errorhandler(404)
# TODO 1a. requests
#      2a. twisted ou algum outro que de pra deixar assincrono
def bypass(e=None, domain=None):
    print(locals())
    print(request)
    print(dir(request))
    #import pdb; pdb.set_trace()
    return '',200


if __name__ == '__main__':
    app.run()
