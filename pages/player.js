let answerGiven = false;
let newQuestion = false;
let currentQuestion = 0;

let ws = null;

function giveAnswer(index) {


    var data = {};

    data["answer"] = index;
    data["playerID"] = document.getElementById("playerID").innerHTML;

    answerGiven = true;

    $.ajax({
        type: "POST",
        url: "./player.html",
        data: data,
        success: function (data) {
            handlePlayerAnswerGiven();

        },
        error: function (error) {
            console.log("Error:");
            handlePlayerAnswerGiven();
        }
    });

}


function handlePlayerAnswerGiven() {
    document.getElementById("pitanjeDiv").style.display = "none";

}


function handlePlayerStatus(status) {


    document.getElementById("playerID").innerHTML = status["player"]["id"];
    document.getElementById("playerUsername").innerHTML = status["player"]["username"];
    document.getElementById("score").innerHTML = status["player"]["score"];


    let currQuestion = status["currQuestion"] + 1;
    document.getElementById("questions").innerHTML = currQuestion + "/" + status["totalQuestions"];


    let gameStatus = status["gameState"]

    if (gameStatus !== 2) {


    } else if (gameStatus === 2 && currQuestion !== currentQuestion) {

        currentQuestion = currQuestion;
        answerGiven = false;
        document.getElementById("pitanjeDiv").style.display = "block";
        document.getElementById("question").innerHTML = status["question"];

        for (let i = 1; i <= 4; i++)
            document.getElementById("answer" + i).innerHTML = status["answers"][i - 1];

    } else if (answerGiven === true) {
        document.getElementById("pitanjeDiv").style.display = "none";

    }
}


function getPlayerStatus() {

    var data = {};

    data["status"] = "1";

    var res = JSON.stringify(data)

    console.log(res);


    $.ajax({
        type: "GET",
        url: "./player.html",
        data: data,
        success: function (data) {
            handlePlayerStatus(data);

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
        gameInit.type = "player";
        gameInit.method = "gameInit";
        gameInit.roomNumber = 1;
        gameInit.username = "Igor";
        ws.send(JSON.stringify(gameInit));
    };

    ws.onmessage = function (evt) {

        console.log("MESSAGE: " + evt.data);

        let received_msg = JSON.parse(evt.data);
        if (received_msg.gameState)
            handlePlayerStatus(received_msg);
        else
            handleScores(received_msg);
    };

    ws.onclose = function () {

        // websocket is closed.
        alert("Connection is closed...");
        ws = null;
    };
} else {
    window.setInterval(function () {
        getPlayerStatus();  //calling every .5 seconds
    }, 200);
}

