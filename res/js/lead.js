let data_list = [];
let word = "";
let index = 0;
let prompted = false;
let userid = "__lead_guest__";
let passwd = "__lead_guest__";

function prev_quiz() {
    if (data_list.length == 1) {
        mdui.snackbar("没有上一个了");
    } else {
        data_list.pop();
        word = data_list[data_list.length - 1]["word"]
        index = data_list[data_list.length - 1]["index"]
        apply_quiz(data_list[data_list.length - 1]["quiz"])
    }
}

function apply_quiz(new_quiz) {
    $("#A").attr("class", "mdui-list-item", "mdui-ripple");
    $("#B").attr("class", "mdui-list-item", "mdui-ripple");
    $("#C").attr("class", "mdui-list-item", "mdui-ripple");
    $("#D").attr("class", "mdui-list-item", "mdui-ripple");
    $("#explanation").html("");
    $("#question").html(new_quiz["question"]);
    $("#A").html("A. " + new_quiz["options"]["A"]);
    $("#B").html("B. " + new_quiz["options"]["B"]);
    $("#C").html("C. " + new_quiz["options"]["C"]);
    $("#D").html("D. " + new_quiz["options"]["D"]);
    prompted = false;
}

function get_quiz(word_index = -1) {
    $.ajax({
        type: 'GET',
        url: "api/get_quiz",
        data:
            (word_index == -1) ?
                ({
                    user_id: userid,
                    password: passwd
                }) : ({
                    word_index: word_index,
                    user_id: userid,
                    password: passwd
                }),
        success: function (result) {
            data_list.push(result)
            word = result["word"]
            index = result["index"]
            apply_quiz(data_list[data_list.length - 1]["quiz"])
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

window.onload = function () {
    get_quiz(-1);
};

function quiz_select(opt) {
    if (opt == data_list[data_list.length - 1]["quiz"]["answer"]) {
        if (!prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_passed",
                data:
                    {
                        word_index: index,
                        user_id: userid,
                        password: passwd
                    },
                error: function (XMLHttpRequest, textStatus, errorThrown) {
                    console.log(XMLHttpRequest.status);
                    console.log(XMLHttpRequest.readyState);
                    console.log(textStatus);
                }
            });
        }
        get_quiz()
        apply_quiz(data_list[data_list.length - 1]["quiz"])
    } else {
        if (!prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_failed",
                data:
                    {
                        word_index: index,
                        user_id: userid,
                        password: passwd
                    },
                error: function (XMLHttpRequest, textStatus, errorThrown) {
                    console.log(XMLHttpRequest.status);
                    console.log(XMLHttpRequest.readyState);
                    console.log(textStatus);
                }
            });
        }
        $("#" + opt).addClass("mdui-color-red");
    }
}

function quiz_prompt(opt) {
    if (!prompted) {
        prompted = true;
        $("#" + opt).addClass("mdui-color-red");
        $.ajax({
            type: 'GET',
            url: "api/quiz_prompt",
            data:
                {
                    word_index: index,
                    user_id: userid,
                    password: passwd
                },
            error: function (XMLHttpRequest, textStatus, errorThrown) {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        $("#" + data_list[data_list.length - 1]["quiz"]["answer"]).addClass("mdui-color-green");
    }
}

function get_explanation(opt) {
    if (!prompted) {
        prompted = true;
        $("#" + opt).addClass("mdui-color-red");
        $.ajax({
            type: 'GET',
            url: "api/explanation",
            data:
                {
                    word_index: index
                },
            success: function (result) {
                if (result["status"] == "success") {
                    $("#explanation").html(result["explanation"]);
                }
            },
            error: function (XMLHttpRequest, textStatus, errorThrown) {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
    }
}

function pass() {
    $.ajax({
        type: 'GET',
        url: "api/pass",
        data:
            {
                word_index: index,
                user_id: userid,
                password: passwd
            },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
    get_quiz()
}


document.getElementById('login-form').addEventListener('submit', event => {
    var uid = document.getElementById("userid").value;
    var p = document.getElementById("passwd").value;
    $.ajax({
        type: 'GET',
        url: "api/login",
        data:
            {
                user_id: uid,
                password: p
            },
        success: function (result) {
            if (result["status"] == "success") {
                userid = uid;
                passwd = p;
            }
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

document.getElementById('search-form').addEventListener('submit', event => {
    var search_word = document.getElementById("search-word").value;
    $.ajax({
        type: 'GET',
        url: "api/search",
        data:
            {
                word_index: search_word,
                user_id: userid,
                password: passwd
            },
        success: function (result) {
            if (result["status"] == "success")
                get_quiz(result["index"])
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
