from flask import Flask, render_template, request, redirect, url_for, session
from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash

app = Flask(__name__)
app.secret_key = "your_secret_key"

# Configure Database
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///users.db"  # SQLite database
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False  # To suppress warning

# Initialize the database
db = SQLAlchemy(app)


# Create User Model (table)
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(150), unique=True, nullable=False)
    password = db.Column(db.String(150), nullable=False)


# Instead of @app.before_first_request, create tables at startup
with app.app_context():
    db.create_all()


# Main route
@app.route("/")
def index():
    return render_template("login.html")


# Login Route
@app.route("/login", methods=["POST"])
def login():
    username = request.form.get("username")
    password = request.form.get("password")

    # Query user from the database
    user = User.query.filter_by(username=username).first()

    if user and check_password_hash(user.password, password):
        session["user"] = user.username
        return redirect(url_for("dashboard"))
    else:
        return redirect(url_for("index"))  # If authentication fails, redirect to login


# Signup Route
@app.route("/signup", methods=["GET", "POST"])
def signup():
    if request.method == "POST":
        username = request.form.get("username")
        password = request.form.get("password")

        # Check if the user already exists
        existing_user = User.query.filter_by(username=username).first()
        if existing_user is None:
            # Hash the password for security
            hashed_password = generate_password_hash(password, method="pbkdf2:sha256")
            new_user = User(username=username, password=hashed_password)
            db.session.add(new_user)
            db.session.commit()
            return redirect(url_for("index"))

    return render_template("signup.html")


# Dashboard route
@app.route("/dashboard")
def dashboard():
    if "user" in session:
        username = session["user"]  # Get the logged-in username from the session
        return render_template("dashboard.html", username=username)
    else:
        return redirect(url_for("index"))


# Logout Route
@app.route("/logout", methods=["POST"])
def logout():
    session.pop("user", None)
    return redirect(url_for("index"))


# Help Route (accessible to anyone)
@app.route("/help")
def help():
    return render_template("help.html")


# Settings Route - Only accessible if logged in
@app.route("/settings", methods=["GET", "POST"])
def settings():
    if "user" not in session:  # Check if user is logged in
        return redirect(url_for("index"))  # If not logged in, redirect to login page

    # Fetch the current user's username from session
    current_username = session["user"]

    if request.method == "POST":
        new_username = request.form.get("new_username")
        new_password = request.form.get("new_password")

        # Update the user's information in the database
        user = User.query.filter_by(username=current_username).first()

        if new_username:
            user.username = new_username
            session["user"] = new_username  # Update the session with new username
        if new_password:
            hashed_password = generate_password_hash(
                new_password, method="pbkdf2:sha256"
            )
            user.password = hashed_password

        db.session.commit()

        return redirect(url_for("dashboard"))

    return render_template("settings.html", username=current_username)


# Analytics Route
@app.route("/analytics")
def analytics():
    if "user" in session:
        return render_template("analytics.html")
    else:
        return redirect(url_for("index"))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
