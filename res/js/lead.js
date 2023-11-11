"use strict"
let current_page = "";

let username = "__lead_guest__";
let passwd = "__lead_guest__";
let userinfo = null;

let quiz_data_list = [];
let quiz_word = "";
let quiz_word_index = 0;
let quiz_prompt_data = null;
let quiz_prompted = false;
let quiz_prompt_panel_isopen = [false, false, false, false];

let memorize_word = "";
let memorize_index = 0;
let memorize_meaning = "";

let search_data = null;
let locate_select_dialog = null;

let settings = null;
let mutationObserver = null;
function register(u_name, email, pwd) {
    $.ajax({
        type: 'GET',
        url: "api/register",
        data:
            {
                username: u_name,
                email: email,
                passwd: pwd
            },
        success: function (result) {
            if (result["status"] == "success") {
                username = u_name;
                passwd = pwd;
                userinfo = result;
                update_account_info();
                mdui.snackbar("注册成功");
                document.getElementById('register-dialog-close').click();
            } else
                mdui.snackbar(result["message"]);
        },
    });
}

function login(u_name, pwd) {
    $.ajax({
        type: 'GET',
        url: "api/login",
        data:
            {
                username: u_name,
                passwd: pwd
            },
        success: function (result) {
            if (result["status"] == "success") {
                username = u_name;
                passwd = pwd;
                userinfo = result;
                update_account_info();
                mdui.snackbar("欢迎回来, " + u_name);
                document.getElementById('login-dialog-close').click();
            } else
                mdui.snackbar(result["message"]);
        },
    });
}

function upload_profile_picture(pic)
{
    $.ajax({
        type: 'POST',
        url: "api/upload_profile_picture?username=" + username + "&passwd=" + passwd,
        dataType: "json",
        processData: false,
        contentType : false,
        data: pic,
        success: function (result) {
            if (result["status"] == "success") {
                $("#account-picture").attr("src", "/userpic/" + result["profile_picture"]);
                userinfo["profile_picture"] = result["profile_picture"];
                update_account_info();
                mdui.snackbar("上传成功");
                document.getElementById('upload-dialog-close').click();
            } else
                mdui.snackbar(result["message"]);
        },
    });
}

function update_account_info()
{
    $("#account-name").html(username);
    $("#account-email").html(userinfo["email"]);
    $("#account-email").removeClass("mdui-hidden");
    if(userinfo["profile_picture"] != '')
        $("#account-picture").attr("src", "/userpic/" + userinfo["profile_picture"]);
    $("#account-info").removeAttr("mdui-dialog");
    $("#account-picture").attr("mdui-dialog", "{target: '#uploadDialog', history: false, modal: true}");
    window.localStorage.setItem("user", JSON.stringify({username: username, passwd: passwd}));
    window.localStorage.setItem("userinfo", JSON.stringify(userinfo));
}


function init_page(with_appbar) {
    with_appbar = typeof with_appbar !== 'undefined' ? with_appbar : true;
    if (with_appbar) {
        $.ajax({
            type: 'GET',
            url: "fragment/appbar.html",
            async: false,
            success: function (result) {
                $("#appbar-placeholder").html(result);
                $("#page-" + current_page).addClass("mdui-list-item-active");
                $("#page-" + current_page + " > div").removeClass("mdui-text-color-black-text");
                mdui.mutation();
            }
        });
        var search_form = document.getElementById("search-form");
        search_form.onsubmit = function (event) {
            var search_word = document.getElementById("search-value").value;
            $.ajax({
                type: 'GET',
                url: "api/search",
                data:
                    {
                        word: search_word,
                        username: username,
                        passwd: passwd
                    },
                success: function (result) {
                    if (result["status"] == "success") {
                        window.sessionStorage.setItem("search_result", JSON.stringify(result));
                        window.location.href = "search.html";
                    } else
                        mdui.snackbar(result["message"]);
                },
            });
            event.preventDefault();
        }

        var login_form = document.getElementById("login-form");
        login_form.onsubmit = function (event) {
            var u_name = document.getElementById("login-user-name").value;
            var pwd = document.getElementById("login-password").value;
            login(u_name, pwd);
            event.preventDefault();
        }

        var register_form = document.getElementById("register-form");
        register_form.onsubmit = function (event) {
            var u_name = document.getElementById("register-user-name").value;
            var email = document.getElementById("register-email").value;
            var pwd = document.getElementById("register-password").value;
            register(u_name, email, pwd);
            event.preventDefault();
        }

        var upload_form = document.getElementById("upload-form");
        upload_form.onsubmit = function (event) {
            var pic = $("#upload-pic")[0].files[0];
            upload_profile_picture(pic);
            event.preventDefault();
        }

        mutationObserver = new MutationObserver(function(mutations) {
            mutations.forEach(function(mutation) {
                    if ($("#toolbar-search").hasClass('mdui-textfield-expanded')) {
                        $(".toolbar-hidden-when-search").addClass("search-bar-hidden");
                        $("#toolbar-search-container").addClass("search-bar-container");
                    } else {
                        $(".toolbar-hidden-when-search").removeClass("search-bar-hidden");
                        $("#toolbar-search-container").removeClass("search-bar-container");
                    }
            });
        });
        mutationObserver.observe($('#toolbar-search')[0], {
            attributes: true,
        });
    }
    var user = JSON.parse(window.localStorage.getItem("user"));
    if(!$.isEmptyObject(user) && user["username"] != "__lead_guest__") {
        username = user["username"];
        passwd = user["passwd"];
        userinfo = JSON.parse(window.localStorage.getItem("userinfo"));
        update_account_info();
    }
    $.ajax({
        type: 'GET',
        url: "api/get_settings",
        data:
            {
                username: username,
                passwd: passwd
            },
        async: false,
        success: function (result) {
            if (result["status"] == "success") {
                settings = result["settings"];
            } else {
                mdui.snackbar(result["message"]);
            }
        }
    });
}

function init_content(page) {
    switch (page) {
        case "plan":
            $.ajax({
                type: 'GET',
                url: "api/get_plan",
                data: {
                    username: username,
                    passwd: passwd
                },
                success: function (result) {
                    if (result["status"] == "success") {
                        if (result["finished_word_count"] == result["planned_word_count"]) {
                            $("#message").html("<h3>今日计划已完成</h3>");
                        } else {
                            $("#finished_word_count").html(result["finished_word_count"]);
                            $("#planned_word_count").html(result["planned_word_count"]);
                        }
                        $("#progress_bar").attr('style', 'width:' + (result["finished_word_count"] / result["planned_word_count"]) * 100 + '%;');
                    } else {
                        mdui.snackbar(result["message"]);
                    }
                }
            });
            break;
        case "marked":
            $.ajax({
                type: 'GET',
                url: "api/get_marked",
                data:
                    {
                        username: username,
                        passwd: passwd
                    },
                success: function (result) {
                    if (result["status"] == "success") {
                        var content = '<div class="mdui-panel" mdui-panel>';
                        for (let i = result["marked_words"].length - 1; i >= 0; i--) {
                            content += marked_explanation_panel(result["marked_words"][i]);
                        }
                        content += '</div>'
                        $("#marked-words").html(content);
                        mdui.mutation();
                        $("#loading").remove();
                        $("#marked-data").removeClass("mdui-hidden");
                    } else {
                        mdui.snackbar(result["message"]);
                    }
                }
            });
            break;
        case "memorize":
            next_word(false);
            var Element = document.getElementById('memorize-swipe');
            var mc = new Hammer(Element);
            mc.on("swiperight", function (ev) {
                prev_word();
                $(".mdui-bottom-nav-fixed").append(
                    '<button class="mdui-fab mdui-ripple swiperight">' +
                    '<i class="mdui-icon material-icons">navigate_before</i></button>')
                $(".swiperight").fadeTo('normal', 0.01,
                    function () {
                        $(this).slideUp('normal', function () {
                            $(this).remove();
                        });
                    });
                ;
            });
            mc.on("swipeleft", function (ev) {
                next_word(true);
                $(".mdui-bottom-nav-fixed").append(
                    '<button class="mdui-fab mdui-ripple swipeleft">' +
                    '<i class="mdui-icon material-icons">navigate_next</i></button>')
                $(".swipeleft").fadeTo('normal', 0.01,
                    function () {
                        $(this).slideUp('normal', function () {
                            $(this).remove();
                        });
                    });
                ;
            });
            break;
        case "passed":
            $.ajax({
                type: 'GET',
                url: "api/get_passed",
                data: {
                    username: username,
                    passwd: passwd
                },
                success: function (result) {
                    if (result["status"] == "success") {
                        $("#passed_word_count").html(result["passed_word_count"]);
                        $("#word_count").html(result["word_count"]);
                        $("#progress_bar").attr('style', 'width:' + (result["passed_word_count"] / result["word_count"]) * 100 + '%;')

                        var content = '<div class="mdui-panel" mdui-panel>';
                        for (let i = result["passed_words"].length - 1; i >= 0; i--) {
                            content += passed_explanation_panel(result["passed_words"][i]);
                        }
                        content += '</div>'
                        $("#passed-words").html(content);
                        mdui.mutation();
                        $("#loading").remove();
                        $("#passed-data").removeClass("mdui-hidden");
                    } else {
                        mdui.snackbar(result["message"]);
                    }
                }
            });
            break;
        case "quiz":
            next_quiz(-1);
            break;
        case "search":
            if (search_data == null)
                search_data = JSON.parse(window.sessionStorage.getItem("search_result"));
            else
                window.sessionStorage.setItem("search_result", search_data);
            var content = '<div class="mdui-panel" mdui-panel>';
            for (var word in search_data["words"])
                content += search_explanation_panel(search_data["words"], word);
            content += '</div>'
            $("#search-result").html(content);
            $("#search-title").html(search_data["message"]);
            mdui.mutation();
            break;
        case "settings":
            for (var s in settings) {
                document.getElementById(s).checked = settings[s];
            }
            $("#loading").remove();
            $("#settings-data").removeClass("mdui-hidden");
            break;
    }
}

function load_body(page)
{
    if(current_page != "") {
        $("#page-" + current_page).removeClass("mdui-list-item-active");
        $("#page-" + current_page + " > div").addClass("mdui-text-color-black-text");
    }
    current_page = page;
    $.ajax({
        type: 'GET',
        url: "fragment/" + page + ".html",
        async: false,
        success: function (result) {
            $("#page-" + page).addClass("mdui-list-item-active");
            $("#page-" + page + " > div").removeClass("mdui-text-color-black-text");
            let doc = new DOMParser().parseFromString(result, 'text/html');
            let bodyClassList = doc.querySelector('body').classList;
            let content = doc.querySelector('#content');
            document.getElementById('content').replaceWith(content);
            document.getElementsByTagName('body')[0].classList = bodyClassList;
            init_content(page);
            if(window.innerWidth < 599)
                document.getElementById('drawer-button')?.click();
        }
    });
}

function init_prompt() {
    $("#explanation").html('<div class="mdui-panel" mdui-panel>' +
        prompt_explanation_panel(quiz_prompt_data, "A", quiz_prompt_panel_isopen[0]) +
        prompt_explanation_panel(quiz_prompt_data, "B", quiz_prompt_panel_isopen[1]) +
        prompt_explanation_panel(quiz_prompt_data, "C", quiz_prompt_panel_isopen[2]) +
        prompt_explanation_panel(quiz_prompt_data, "D", quiz_prompt_panel_isopen[3]) +
        '</div>');
    mdui.mutation();
}

function init_explanation(word_index) {
    $.ajax({
        type: 'GET',
        url: "api/get_explanation",
        data:
            {
                word_index: word_index
            },
        success: function (result) {

            if (result["status"] == "success") {
                $("#explanation-" + word_index).html(result["explanation"]);
            } else {
                mdui.snackbar(result["message"])
            }
        }
    });
}

function update_settings() {
    var settings_list = document.getElementsByClassName("setting");
    for (var s = 0; s < settings_list.length; s++)
        settings[settings_list[s].id] = settings_list[s].checked;
    $.ajax({
        type: 'POST',
        url: "api/update_settings?username=" + username + "&passwd=" + passwd,
        contentType: "application/json",
        dataType: "json",
        data: JSON.stringify(settings),
        success: function (result) {
            if (result["status"] != "success") {
                mdui.snackbar(result["message"]);
            }
        }
    });
}

function save_quiz_prompt_panel_status() {
    var panels = document.getElementsByClassName("mdui-panel-item");
    quiz_prompt_panel_isopen[0] = panels[0].classList.length > 1;
    quiz_prompt_panel_isopen[1] = panels[1].classList.length > 1;
    quiz_prompt_panel_isopen[2] = panels[2].classList.length > 1;
    quiz_prompt_panel_isopen[3] = panels[3].classList.length > 1;
}

function update_memorize_data(result) {
    memorize_word = result["word"]["word"];
    if (settings["memorize_autoplay"])
        speak(memorize_word)
    memorize_index = result["word"]["word_index"];
    memorize_meaning = result["word"]["meaning"];
    $("#explanation").html(result["content"]);
    $("#search-locate-value").val("");
    $("#search-locate-value").attr("placeholder", memorize_word);
    $('#locate-value').attr('value', memorize_index)
    mdui.updateSliders();
}

function set_memorize_word(word_index) {
    $.ajax({
        type: 'GET',
        url: "api/set_memorize_word",
        data:
            {
                word_index: word_index,
                username: username,
                passwd: passwd
            },
        success: function (result) {

            if (result["status"] == "success")
                update_memorize_data(result);
            else
                mdui.snackbar(result["message"]);
        }
    });
}


function prev_word() {
    $.ajax({
        type: 'GET',
        url: "api/prev_memorize_word",
        data: {
            username: username,
            passwd: passwd
        },
        success: function (result) {

            if (result["status"] == "success")
                update_memorize_data(result);
            else
                mdui.snackbar(result["message"]);
        }
    });
}

function next_word(next) {
    next = typeof next !== 'undefined' ? next : false;
    $.ajax({
        type: 'GET',
        url: "api/memorize_word",
        data:
            {
                next: next,
                username: username,
                passwd: passwd
            },
        success: function (result) {

            if (result["status"] == "success")
                update_memorize_data(result);
            else
                mdui.snackbar(result["message"]);
        }
    });
}

function mark_word(word_index) {
    $.ajax({
        type: 'GET',
        url: "api/mark_word",
        data:
            {
                word_index: word_index,
                username: username,
                passwd: passwd
            },
        success: function (result) {

            if (result["status"] == "success")
                mdui.snackbar("收藏成功");
            else
                mdui.snackbar(result["message"]);
        }
    });
}

function unmark_word(word_index) {
    $.ajax({
        type: 'GET',
        data:
            {
                word_index: word_index,
                username: username,
                passwd: passwd
            },
        url: "api/unmark_word",
        success: function (result) {

            if (result["status"] == "success")
                mdui.snackbar("取消收藏成功");
            else
                mdui.snackbar(result["message"]);
        }
    });
}

function prev_quiz() {
    if (quiz_data_list.length == 1) {
        mdui.snackbar("没有上一个了");
    } else {
        quiz_data_list.pop();
        quiz_word = quiz_data_list[quiz_data_list.length - 1]["quiz_word"];
        quiz_word_index = quiz_data_list[quiz_data_list.length - 1]["word_index"];
        apply_quiz(quiz_data_list[quiz_data_list.length - 1]["quiz"]);
    }
}

function apply_quiz(new_quiz) {
    $("#A").parent().removeClass("mdui-color-red");
    $("#A").parent().removeClass("mdui-color-light-green");
    $("#B").parent().removeClass("mdui-color-red");
    $("#B").parent().removeClass("mdui-color-light-green");
    $("#C").parent().removeClass("mdui-color-red");
    $("#C").parent().removeClass("mdui-color-light-green");
    $("#D").parent().removeClass("mdui-color-red");
    $("#D").parent().removeClass("mdui-color-light-green");
    $("#explanation").html("");
    $("#question").html(new_quiz["question"]);
    $("#A").html("A. " + new_quiz["options"]["A"]);
    $("#B").html("B. " + new_quiz["options"]["B"]);
    $("#C").html("C. " + new_quiz["options"]["C"]);
    $("#D").html("D. " + new_quiz["options"]["D"]);
    quiz_prompted = false;
    quiz_prompt_panel_isopen = [false, false, false, false];
}

function next_quiz(word_index) {
    word_index = typeof word_index !== 'undefined' ? word_index : -1;
    $.ajax({
        type: 'GET',
        url: "api/get_quiz",
        data:
            (word_index == -1) ?
                ({
                    username: username,
                    passwd: passwd
                })
                :
                ({
                    word_index: word_index,
                    username: username,
                    passwd: passwd
                }),
        success: function (result) {
            quiz_data_list.push(result)
            quiz_word = result["word"]["word"]
            quiz_word_index = result["word"]["word_index"]
            apply_quiz(quiz_data_list[quiz_data_list.length - 1]["quiz"])
        }
    });
}

function quiz_select(opt) {
    if (opt == quiz_data_list[quiz_data_list.length - 1]["quiz"]["answer"]) {
        if (!quiz_prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_passed",
                data:
                    {
                        word_index: quiz_word_index,
                        username: username,
                        passwd: passwd
                    },
            });
        }
        next_quiz()
        apply_quiz(quiz_data_list[quiz_data_list.length - 1]["quiz"])
    } else {
        if (!quiz_prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_failed",
                data:
                    {
                        word_index: quiz_word_index,
                        username: username,
                        passwd: passwd
                    },
            });
        }
        $("#" + opt).parent().addClass("mdui-color-red");
    }
}

function get_explanation_panel(title, summary, body, actions, open, open_actions) {
    actions = typeof actions !== 'undefined' ? actions : "";
    open_actions = typeof open_actions !== 'undefined' ? open_actions : "";
    open = typeof open !== 'undefined' ? open : true;
    let ret = '';
    if (open)
        ret += '<div class="mdui-panel-item mdui-panel-item-open">';
    else
        ret += '<div class="mdui-panel-item" onclick="' + open_actions + '">';
    ret += '<div class="mdui-panel-item-header">' +
        '<div class="mdui-panel-item-title">' + title + '</div>' +
        '<div class="mdui-panel-item-summary">' + summary + '</div>' +
        '<i class="mdui-panel-item-arrow mdui-icon material-icons">keyboard_arrow_down</i>' +
        '</div><div class="mdui-panel-item-body">' + body;
    if (actions != "") {
        ret += '<div class="mdui-float-right">' + actions + '</div>';
    }
    ret += '</div></div>';
    return ret;
}

function prompt_explanation_panel(result, opt, open) {
    let actions = "";
    if (result[opt]["is_marked"]) {
        actions = '' +
            '<button class=\"mdui-btn mdui-ripple\" onclick=\"save_quiz_prompt_panel_status();unmark_word('
            + quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"][opt] + ');' +
            'quiz_prompt_data[\'' + opt + '\'][\'is_marked\']=false;init_prompt()\"><i class="mdui-icon material-icons">delete</i>取消收藏</button>';
    } else {
        actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"save_quiz_prompt_panel_status();mark_word('
            + quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"][opt] + ');' +
            'quiz_prompt_data[\'' + opt + '\'][\'is_marked\']=true;init_prompt()\"><i class="mdui-icon material-icons">star</i>收藏</button>';
    }
    return get_explanation_panel(opt, quiz_data_list[quiz_data_list.length - 1]["quiz"]["options"][opt],
        result[opt]["explanation"], actions, open);
}

function search_explanation_panel(words, pos) {
    let actions = "";
    if (words[pos]["is_marked"]) {
        actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"unmark_word('
            + words[pos]["word"]["word_index"] + ');' +
            'search_data[\'words\'][' + pos + '][\'is_marked\']=false;' +
            'init_search_result()\"><i class="mdui-icon material-icons">delete</i>取消收藏</button>';
    } else {
        actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"mark_word('
            + words[pos]["word"]["word_index"] + ');' +
            'search_data[\'words\'][' + pos + '][\'is_marked\']=true;' +
            'init_search_result()\"><i class="mdui-icon material-icons">star</i>收藏</button>';

    }
    return get_explanation_panel(words[pos]["word"]["word"],
        words[pos]["word"]["meaning"],
        '<span id="explanation-' + words[pos]["word"]["word_index"] + '">' +
        '<div class="mdui-progress"><div class="mdui-progress-indeterminate"></div></div></span>',
        actions, false, 'init_explanation(' + words[pos]["word"]["word_index"] + ')');
}


function marked_explanation_panel(word) {
    return get_explanation_panel(word["word"]["word"],
        word["word"]["meaning"],
        '<span id="explanation-' + word["word"]["word_index"] + '">' +
        '<div class="mdui-progress"><div class="mdui-progress-indeterminate"></div></div></span>',
        '<button class=\"mdui-btn mdui-ripple\" onclick=\"unmark_word('
        + word["word_index"] + ');$(this).parent().parent().parent().fadeTo(\'normal\', 0.01, function(){$(this).slideUp(\'normal\', function() {$(this).remove();});});;\">' +
        '<i class="mdui-icon material-icons">delete</i>取消收藏</button>',
        false, 'init_explanation(' + word["word"]["word_index"] + ')');
}

function passed_explanation_panel(word) {
    var actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"renew('
        + word["word"]["word_index"] + ');$(this).parent().parent().parent().' +
        'fadeTo(\'normal\', 0.01, function(){$(this).slideUp(\'normal\', function() {$(this).remove();});});' +
        'var cnt = parseInt($(\'#passed_word_count\')[0].innerText) - 1;\n' +
        'var width = (cnt / parseInt($(\'#word_count\')[0].innerText)) * 100 + \'%\';' +
        '$(\'#progress_bar\').attr(\'style\', \'width:\' + width);' +
        '$(\'#passed_word_count\').html(cnt);\">' +
        '<i class="mdui-icon material-icons">replay</i>重新背</button>';
    return get_explanation_panel(word["word"]["word"],
        word["word"]["meaning"],
        '<span id="explanation-' + word["word"]["word_index"] + '">' +
        '<div class="mdui-progress"><div class="mdui-progress-indeterminate"></div></div></span>',
        actions, false, 'init_explanation(' + word["word"]["word_index"] + ')');
}


function quiz_prompt(opt) {
    if (!quiz_prompted) {
        quiz_prompted = true;
        $("#" + opt).parent().addClass("mdui-color-red");
        $.ajax({
            type: 'GET',
            url: "api/quiz_prompt",
            data:
                {
                    A_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["A"],
                    B_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["B"],
                    C_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["C"],
                    D_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["D"],
                    word_index: quiz_word_index,
                    username: username,
                    passwd: passwd
                },
            success: function (result) {

                quiz_prompt_data = result;
                if (result["status"] == "success") {
                    init_prompt()
                } else {
                    mdui.snackbar(result["message"])
                }
            },
        });
        $("#" + quiz_data_list[quiz_data_list.length - 1]["quiz"]["answer"]).parent().addClass("mdui-color-light-green");
    }
}

function pass(word_index) {
    $.ajax({
        type: 'GET',
        url: "api/pass",
        data:
            {
                word_index: word_index,
                username: username,
                passwd: passwd
            }
    });
}

function renew(word_index) {
    $.ajax({
        type: 'GET',
        url: "api/renew",
        data:
            {
                word_index: word_index,
                username: username,
                passwd: passwd
            }
    });
}

function speak(word) {
    var url = "http://dict.youdao.com/dictvoice?type=0&audio=" + encodeURI(word.replaceAll(' ', '-'));
    var n = new Audio(url);
    n.play();
}

function clear_records() {
    $.ajax({
        type: 'GET',
        url: "api/clear_records",
        data:{
            username: username,
            passwd: passwd
        },
        success: function (result) {
            quiz_prompt_data = result;
            if (result["status"] == "success") {
                init_passed();
                mdui.snackbar("已清除所有记录");
            } else {
                mdui.snackbar(result["message"]);
            }
        }
    });
}

function clear_marks() {
    $.ajax({
        type: 'GET',
        url: "api/clear_marks",
        data:{
            username: username,
            passwd: passwd
        },
        success: function (result) {
            quiz_prompt_data = result;
            if (result["status"] == "success") {
                init_marked();
                mdui.snackbar("已清除所有收藏");
            } else {
                mdui.snackbar(result["message"]);
            }
        }
    });
}

function locate_word() {
    var word_index = document.getElementById("locate-value").value;
    $.ajax({
        type: 'GET',
        async: false,
        url: "api/get_word",
        data:
            {
                word_index: word_index
            },
        success: function (result) {

            if (result["status"] == "success") {
                $("#search-locate-value").val("");
                $("#search-locate-value").attr("placeholder", result["word"]);
                if ($("#search-locate-textfield").hasClass("mdui-textfield-invalid"))
                    $("#search-locate-textfield").removeClass("mdui-textfield-invalid");
            } else {
                mdui.snackbar(result["message"])
            }
        }
    });
}

function locate_word_set() {
    if ($("#search-locate-value").val() != "") {
        $.ajax({
            type: 'GET',
            url: "api/search",
            data:
                {
                    word: $("#search-locate-value").val(),
                    username: username,
                    passwd: passwd
                },
            success: function (result) {

                if (result["status"] == "success") {
                    var content = "<div class=\"mdui-dialog\"><div class=\"mdui-dialog-title\">选择</div><div class=\"mdui-dialog-content\"><div class=\"mdui-list\">";
                    for (var word in result["words"])
                        content += "<a class=\"mdui-list-item mdui-ripple\" onclick=\"set_memorize_word("
                            + result["words"][word]["word"]["word_index"] + ");locate_select_dialog.close()\">"
                            + result["words"][word]["word"]["word"] + " " + result["words"][word]["word"]["meaning"] + "</a>"
                    content += '</div></div></div>'
                    document.getElementById("locateDialog-close").click();
                    locate_select_dialog = new mdui.Dialog(content, {"modal": true});
                    locate_select_dialog.open();
                } else {
                    if (!$("#search-locate-textfield").hasClass("mdui-textfield-invalid"))
                        $("#search-locate-textfield").addClass("mdui-textfield-invalid");
                }
            },
        });
    } else {
        set_memorize_word(document.getElementById('locate-value').value);
        document.getElementById("locateDialog-close").click();
    }
}

function locate_verify() {
    var word = $("#search-locate-value").val();
    if (word == "") return;
    $.ajax({
        type: 'GET',
        url: "api/search",
        data:
            {
                word: word,
                username: username,
                passwd: passwd
            },
        success: function (result) {

            if (result["status"] == "success") {
                if ($("#search-locate-textfield").hasClass("mdui-textfield-invalid"))
                    $("#search-locate-textfield").removeClass("mdui-textfield-invalid");
            } else if (!$("#search-locate-textfield").hasClass("mdui-textfield-invalid"))
                $("#search-locate-textfield").addClass("mdui-textfield-invalid");
        },
    });
}