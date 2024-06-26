This Steam-Bot only has a simple CLI... for now?

Also, some commands are very basic. This is primarly a UI so I can test framework functionality, so I'll often add such simple commands before even starting on more substantial functionality that utilizies these basic building blocks.

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
Instead of an account name, you can also provide a `@groupname:` representing all accounts in that group, or `*:` which addresses all active accounts. This lets you run the same command on multiple accounts.

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

* `[<accountname>:] list-games [--adult] {--early-access] [--playtime] [<regex>]`\
   list games owned by the account. The number displayed is the `app-id`.\
   If a regular expression pattern is provided, only lists games matching the pattern.\
   Note: if you don't want to bother with regexes, just typing a string will usually just find games with that text in their name.\
   `--adult` and `--early-access` options will only list those.\
   `--playtime` option will sort by playtime instead of game name.
* `[<accountname>:] play-game <app-id>`\
  `[<accountname>:] stop-game <app-id>`\
  start/stop "playing" that specified game
* `[<accountname>:] add-license <package-id>`\
  add a free license (F2P, demo) to the account
* `[<accountname>:] add-app <app-id>`\
  add a free license (F2P, demo) to the account
* `[<accountname>:] clear-queue`\
  clear one discovery queue.
* `[<accountname>:] sale-sticker`\
  claim a sale sticker, if available.
* `[<accountname>:] view-stream url`\
  `[<accountname>:] stop-stream url`\
  Start/stop "watching" a stream on a given Steam page. Note that while this exists to get drops, it does not detect
  when the drops have been given.

# Complex actions

* `[<accountname>:] sale-queue`\
  clear all sale-queues.
* `[<accountname>:] sale-event`\
  Performs supported sale-event activities. Currently, this means:
  * clearing sale event discovery queues
  * claiming a sale-sticker

# Inventory

* `[<accountname>:] list-inventory [--tradable] [<regex>]`\
  lists items from the inventory

# Trading

* `[<accountname>:] list-tradeoffers`\
  list incoming trade offers

* `[<accountname>:] send-inventory [<accountname>]`\
  sends (all/the first 100) tradable items from the inventory to the other account.\
  Note that the recipient account must also be configured on this bot, at least for now.\
  Also note that you will have confirm the trade as usual; the bot doesn't do that (and likely never will).\
  A `send-inventory-recipient` setting is provided as the default recipient.

* `[<accountname>:] accept-trade <tradeofferid>`\
  accepts a trade.\
  Note that, for some reason, I haven't added a command to list incoming trades. It will get and print the list once on startup, though.

* `[<accountname>:] decline-trade <tradeofferid>`\
  declines a trade.

# (Persistent) settings

* `[<accountname>:] set`\
  list all settings and their current values

* `[<accountname>:] set name value`\
  change a setting. This will only accept valid values.

* `[<accountname>:] set name`\
  reset the setting to default value.

Current settings are:

* `card-farmer-enable`: to enable/disable card farming.\
  Accepted values are various `bool` representations, such as `on`, `off`, `yes`, `no` etc.

* `send-inventory-recipient`: the default account for the `send-inventory` command.`\
  Accepted values are valid bot account names.

# Group management

* `create-group <groupname> <accountname> [<accountname> ...]`\
  create a new group

* `add-group <groupname> <accountname> [<accountname> ...]`\
  add more accounts to a group

* `remove-group <groupname> <accountname> [<accountname> ...]`\
  remove accounts from group

* `list-groups`\
  show groups and their members
