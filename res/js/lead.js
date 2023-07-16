var quiz;
var word;
var pronunciation;
var pos;
var prompted = false;
var token = "abc";
function get_quiz(word_pos = -1)
{
    $("#A").attr("class", "mdui-list-item", "mdui-ripple");
    $("#B").attr("class", "mdui-list-item", "mdui-ripple");
    $("#C").attr("class", "mdui-list-item", "mdui-ripple");
    $("#D").attr("class", "mdui-list-item", "mdui-ripple");
    $.ajax({
        type: 'GET',
        url: word_pos == -1 ? ("get_quiz?token=" + token) : ("get_quiz?token=" + token + "&pos=" + word_pos),
        success: function (result)
        {
            quiz = result["quiz"]
            word = result["word"]
            pronunciation = result["pronunciation"]
            pos = result["pos"]
            $("#word").html(word);
            $("#pronunciation").html(pronunciation);
            $("#A").html("A. " + quiz["options"]["A"]);
            $("#B").html("B. " + quiz["options"]["B"]);
            $("#C").html("C. " + quiz["options"]["C"]);
            $("#D").html("D. " + quiz["options"]["D"]);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown)
        {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
    prompted = false;
}
window.onload = function(){get_quiz(-1)};
function quiz_select(opt)
{
    if(opt == quiz["answer"]) {
        if(!prompted)
        {
            $.ajax({
                type: 'GET',
                url: "quiz_passed?pos=" + pos + "&token=" + token,
                error: function (XMLHttpRequest, textStatus, errorThrown) {
                    console.log(XMLHttpRequest.status);
                    console.log(XMLHttpRequest.readyState);
                    console.log(textStatus);
                }
            });
        }
        get_quiz()
    }
    else
    {
        $.ajax({
            type: 'GET',
            url: "quiz_failed?pos=" + pos + "&token=" + token,
            error: function (XMLHttpRequest, textStatus, errorThrown)
            {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        $("#" + opt).addClass("mdui-color-red");
    }
}

function quiz_prompt(opt)
{
    prompted = true;
    $("#" + opt).addClass("mdui-color-red");
    $.ajax({
        type: 'GET',
        url: "quiz_prompted?pos=" + pos + "&token=" + token,
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
    $("#" + quiz["answer"]).addClass("mdui-color-green");
}

function pass()
{
    $.ajax({
        type: 'GET',
        url: "pass?pos=" + pos + "&token=" + token,
        error: function (XMLHttpRequest, textStatus, errorThrown)
        {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
    get_quiz()
}

function pronounce()
{
    const synth = window.speechSynthesis;
    const ssu = new SpeechSynthesisUtterance(word);
    ssu.lang = "en-US";
    synth.speak(ssu);
}


function load_login() {
    document.getElementById('login-form').addEventListener('submit', event => {
        var userid = document.getElementById("userid").value;
        var passwd = document.getElementById("passwd").value;
        $.ajax({
            type: 'GET',
            url: "login?userid=" + userid + "&passwd=" + passwd,
            success: function (result) {
                if (result["status"] == "success")
                    token = result["token"];
                mdui.snackbar({message: result["message"]});
            },
            error: function (XMLHttpRequest, textStatus, errorThrown) {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        event.preventDefault();
    });
}

function load_search() {
    document.getElementById('search-form').addEventListener('submit', event => {
        var search_word = document.getElementById("search-word").value;
        $.ajax({
            type: 'GET',
            url: "search?word=" + search_word,
            success: function (result) {
                if (result["status"] == "success")
                    get_quiz(result["pos"])
                mdui.snackbar({message: result["message"]});
            },
            error: function (XMLHttpRequest, textStatus, errorThrown) {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        event.preventDefault();
    });
}
