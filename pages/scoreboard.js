let newQuestion = false;
let currentQuestion = "";

let gameStarted = false;

let time;
let timer;

let ws;

function handleTimer() {

    document.getElementById("countdown").innerHTML = time;

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
            document.getElementById("answer1").style.background = "green";
        if (correctAnsIndex === 1)
            document.getElementById("answer2").style.background = "green";
        if (correctAnsIndex === 2)
            document.getElementById("answer3").style.background = "green";
        if (correctAnsIndex === 3)
            document.getElementById("answer4").style.background = "green";
    } else {
        document.getElementById("answer1").style.background = "";
        document.getElementById("answer2").style.background = "";
        document.getElementById("answer3").style.background = "";
        document.getElementById("answer4").style.background = "";

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

    if ("WebSocket" in window) {
        let gameStart = {};
        gameStart.type = "scoreboard";
        gameStart.method = "gameStart";
        ws.send(JSON.stringify(gameStart));
    } else {

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

}

function getScores() {

    if ("WebSocket" in window) {
        let getScores = {};
        getScores.type = "scoreboard";
        getScores.method = "getScores";
        ws.send(JSON.stringify(getScores));
    } else {

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

    ws = new WebSocket("ws://192.168.1.2:8080/webSocket");

    ws.onopen = function () {

        let gameInit = {};
        gameInit.type = "scoreboard";
        gameInit.method = "gameInit";
        gameInit.numberOfPlayers = 1;
        ws.send(JSON.stringify(gameInit));
    };

    ws.onmessage = function (evt) {
        let received_msg = JSON.parse(evt.data);
        if (received_msg.gameState)
            handleStatus(received_msg);
        else
            handleScores(received_msg);
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

