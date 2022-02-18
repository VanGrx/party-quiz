let newQuestion = false;
let currentQuestion = "";

let gameStarted = false;

let time;
let timer;

function handleTimer() {

    document.getElementById("countdown").innerHTML = time;
    console.log("TIME " + time);

    time = time - 1;

    if (time < 0) {
        clearInterval(timer);
    }

}


function handleScores(scores) {

    document.getElementById("scoresDiv").style.display = "block";
    document.getElementById("scoresDiv").innerHTML = "";

    for (let i in scores) {
        $('<div id="playerScore' + i + '" class="playerScore" style="text-align:left;">' + scores[i].username + ' <span style="float:right;">' + scores[i].score + '</span><\/div>').appendTo("#scoresDiv");
    }

}


function handleStatus(status) {

    let playerText = status.playersEntered + "/" + status.playerNumber;
    let currQuestion = status.currQuestion + 1;
    let questionText = currQuestion + "/" + status.totalQuestions;
    let gameState = status.gameState;
    let gameStateText = "Not Ready";

    if (gameState === 1)
        gameStateText = "Game created";

    if (gameState === 2)
        gameStateText = "Game playing";

    if (gameState === 3) {

        gameStateText = "Game paused";
        getScores();

        let correctAnsIndex = status.correctAnswer;

        if (correctAnsIndex === 0)
            document.getElementById("answer1").style.color = "green";
        if (correctAnsIndex === 1)
            document.getElementById("answer2").style.color = "green";
        if (correctAnsIndex === 2)
            document.getElementById("answer3").style.color = "green";
        if (correctAnsIndex === 3)
            document.getElementById("answer4").style.color = "green";
    } else {
        document.getElementById("answer1").style.color = "black";
        document.getElementById("answer2").style.color = "black";
        document.getElementById("answer3").style.color = "black";
        document.getElementById("answer4").style.color = "black";

    }
    if (gameState === 4)
        gameStateText = "Game finished";

    document.getElementById("gameID").innerHTML = status.gameID;
    document.getElementById("playersConnected").innerHTML = playerText;
    document.getElementById("questions").innerHTML = questionText;
    document.getElementById("gameState").innerHTML = gameStateText;


    newQuestion = (currentQuestion !== currQuestion);


    if (status.gameState === 6 && gameStarted === false) {

        startGame();
        gameStarted = true;

    } else if (status.gameState === 2 && newQuestion) {

        currentQuestion = currQuestion;

        document.getElementById("pitanjeDiv").style.display = "block";
        document.getElementById("question").innerHTML = status.question;
        document.getElementById("answer1").innerHTML = status.answers[0];
        document.getElementById("answer2").innerHTML = status.answers[1];
        document.getElementById("answer3").innerHTML = status.answers[2];
        document.getElementById("answer4").innerHTML = status.answers[3];

        time = 20;
        clearInterval(timer);
        handleTimer();
        timer = setInterval(function () {
            handleTimer();
        }, 1000);

        getScores();

    } else if (status.gameState === 4) {
        document.getElementById("pitanjeDiv").style.display = "none";
        getScores();

    }

}

function startGame() {

    var data = {};

    data.gameStart = "1";

    $.ajax({
        type: "GET",
        url: "./scoreboard.html",
        data: data,
        success: function (data) {
            console.log("GAME STARTED!");

        },
        error: function (error) {
            console.log("Error:");
            console.log(error);
        }
    });

}

function getScores() {

    var data = {};

    data.scores = "1";

    console.log("Calling scores");

    $.ajax({
        type: "GET",
        url: "./scoreboard.html",
        data: data,
        success: function (data) {
            console.log("Calling scores returned");

            handleScores(data);

        },
        error: function (error) {
            console.log("Error:");
            console.log(error);
        }
    });

}


function getStatus() {

    var data = {};


    data.status = "1";


    $.ajax({
        type: "GET",
        url: "./scoreboard.html",
        data: data,
        success: function (data) {
            handleStatus(data);

        },
        error: function (error) {
            console.log("Error:");
            console.log(error);
        }
    });

}

if ("WebSocket" in window) {
    alert("WebSocket is supported by your Browser!");

    let ws = new WebSocket("ws://192.168.1.2:8080/webSocket");

    ws.onopen = function () {

        alert("Message is sending...");
        let grki = {};
        grki.type = "scoreboard";
        grki.method = "gameInit";
        grki.numberOfPlayers = 1;
        ws.send(JSON.stringify(grki));
        alert("Message is sent: " + JSON.stringify(grki));
    };

    ws.onmessage = function (evt) {
        var received_msg = JSON.parse(evt.data);
        for (var k in received_msg)
            alert(k + ": " + received_msg[k]);
        handleStatus(received_msg);
    };

    ws.onclose = function () {

        // websocket is closed.
        alert("Connection is closed...");
    };
} else {

    // The browser doesn't support WebSocket
    alert("WebSocket NOT supported by your Browser!");
    window.setInterval(function () {
        getStatus(); //calling every .5 seconds
    }, 200);
}

