from flask import Flask, render_template

app = Flask(__name__)


# Test route
@app.route("/test")
def test_route():
    print("Serving test route")
    return "Test route is working!"


# Main route
@app.route("/")
def index():
    print("Serving index.html")
    return render_template("index.html")


if __name__ == "__main__":
    app.run(debug=True)
