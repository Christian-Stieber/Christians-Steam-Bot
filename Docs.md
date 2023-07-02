This Steam-Bot only has a simple CLI... for now?

# Activating command mode

Normally, the bot just outputs whatever messages it wants to tell to the screen.

If you want to enter a command, hit the RETURN or TAB key to enter command mode. This will display a prompt, and block the normal screen output.

When you're done, just press RETURN to submit an empty command.

Note:
* normal bot output will be kept and printed after you leave command mode
* the same applies to input prompts, like requests for passwords -- you won't see them in command mode

# General command syntax

A command consists of words that are separated by spaces. If you wish to include spaces in a word, you can either quote the word as in `"this is one word"` or use the `\` character to elimnate any special meaning of the character following it, as in `this\ is\ a\ single\ word`.

If the first word of a command ends with a `:`, as in `name:`, it designates the name of the Steam account to use for this command. There is also a current account that will be used if none is specified on the command. Not every command requires an account, but many commands do.

The next word will be the actual command name, and additional words will be used as parameters as necessary for that command. Examples:
   `account: list-games neptunia`
   `list-games neptunia`

# Basic commands

* `help`\
outputs the list of commands
* `status`\
gives a list of known account names, and their status in the bot
* `create <accountname>`\
  creates a bot for the given accountname, and makes the account name current. You'll probably want to leave command node to see prompts for passwords and steamguard.
* `[<accountname>:] launch`\
  launches an existing bot, and makes it current
* `[<accountname>:] select`\
  sets a new current account
* `[<accountname>:] quit`\
   quit the bot running the account
* `EXIT`\
  quit the entire software

# Basic actions

* `[<accountname>:] list-games [<regex>]`\
   list games owned by the account. The number displayed is the `app-id`.\
   If a regular expression pattern is provided, only lists games matching the pattern.\
   Note: if you don't want to bother with regexes, just typing a string will usually just find games with that text in their name.
* `[<accountname>:] play-game <app-id>`\
  `[<accountname>:] stop-game <app-id>`\
  start/stop "playing" that specified game
* `[<accountname>] add-license <app-id>`\
  add a free license (F2P, demo) to the account
* `[<accountname>] clear-queue`\
  clear one discovery queue.\
  You might want to use the `sale-event` command instead.

# Complex actions

* `[<accountname>] sale-event`\
  Performs supported sale-event activities. Currently, this means:
  * clearing sale event discovery queues
  * claiming a sale-sticker
  