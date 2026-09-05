# ft_irc — Complete Bonus Implementation Guide

> A step-by-step, no-prior-knowledge-assumed guide to building the two bonus
> requirements of the 42 `ft_irc` subject: **file transfer** and **a bot**.
>
> Written against **subject version 10.0** (`en.subject.pdf`, in this repo) and
> against this repository's actual code as of commit `1535948`. Every file path,
> function name, line reference and subject quote below is real — you can
> `grep` for the code and page-check every quote.

---

## Table of contents

- [Part 0 — What the subject actually says](#part-0--what-the-subject-actually-says)
  - [0.1 The bonus text, verbatim](#01-the-bonus-text-verbatim)
  - [0.2 The rule that catches everyone](#02-the-rule-that-catches-everyone)
  - [0.3 The trap: "You must not develop an IRC client"](#03-the-trap-you-must-not-develop-an-irc-client)
  - [0.4 The allowed-functions whitelist](#04-the-allowed-functions-whitelist)
  - [0.5 What we are going to build](#05-what-we-are-going-to-build)
  - [0.6 How much time this takes](#06-how-much-time-this-takes)
- [Part 1 — Pre-flight: fix the mandatory part first](#part-1--pre-flight-fix-the-mandatory-part-first)
  - [1.0 There is no README.md (blocker)](#10-there-is-no-readmemd-blocker)
  - [1.1 The use-after-free (blocker)](#11-the-use-after-free-blocker)
  - [1.2 The poll-loop fd skip (blocker)](#12-the-poll-loop-fd-skip-blocker)
  - [1.3 Silent disconnects never reach the channel (blocker)](#13-silent-disconnects-never-reach-the-channel-blocker)
  - [1.4 Unregistered clients can run any command (blocker)](#14-unregistered-clients-can-run-any-command-blocker)
  - [1.5 Malformed numeric replies](#15-malformed-numeric-replies)
  - [1.6 Malformed JOIN echo](#16-malformed-join-echo)
  - [1.7 No comma-separated channel list in JOIN](#17-no-comma-separated-channel-list-in-join)
  - [1.8 Unbounded input buffer](#18-unbounded-input-buffer)
  - [1.9 Sending without POLLOUT (blocker — explicit grade-0 clause)](#19-sending-without-pollout-blocker--explicit-grade-0-clause)
  - [1.10 Smaller conformance gaps](#110-smaller-conformance-gaps)
  - [1.11 Subject-compliance audit](#111-subject-compliance-audit)
  - [1.12 Pre-flight checklist](#112-pre-flight-checklist)
- [Part 2 — File transfer (DCC)](#part-2--file-transfer-dcc)
  - [2.1 What "handle file transfer" means](#21-what-handle-file-transfer-means)
  - [2.2 How DCC SEND actually works](#22-how-dcc-send-actually-works)
  - [2.3 The exact wire format](#23-the-exact-wire-format)
  - [2.4 What your server must not do](#24-what-your-server-must-not-do)
  - [2.5 Auditing your PRIVMSG path for DCC-safety](#25-auditing-your-privmsg-path-for-dcc-safety)
  - [2.6 Making it demoable: the /dcc test](#26-making-it-demoable-the-dcc-test)
  - [2.6b Complete worked example — transfer a real file with only nc](#26b-complete-worked-example--transfer-a-real-file-with-only-nc)
  - [2.7 Optional: DCC-aware logging](#27-optional-dcc-aware-logging)
  - [2.8 The stronger option: bot-mediated transfer](#28-the-stronger-option-bot-mediated-transfer)
- [Part 3 — The bot: design and architecture](#part-3--the-bot-design-and-architecture)
  - [3.1 What the bot must do](#31-what-the-bot-must-do)
  - [3.2 The bot is a client — the protocol from the other side](#32-the-bot-is-a-client--the-protocol-from-the-other-side)
  - [3.3 Files and structure](#33-files-and-structure)
  - [3.4 The connection lifecycle](#34-the-connection-lifecycle)
  - [3.5 Reusing the server's parser](#35-reusing-the-servers-parser)
  - [3.6 The fallback: a server-side bot](#36-the-fallback-a-server-side-bot)
- [Part 4 — Building the bot, step by step](#part-4--building-the-bot-step-by-step)
  - [4.1 Arguments and entry point](#41-arguments-and-entry-point)
  - [4.2 Opening the connection](#42-opening-the-connection)
  - [4.3 The poll loop](#43-the-poll-loop)
  - [4.4 Output: queueLine and flushOutput](#44-output-queueline-and-flushoutput)
  - [4.5 Input: reading and framing](#45-input-reading-and-framing)
  - [4.6 The registration handshake](#46-the-registration-handshake)
  - [4.7 Dispatching server messages](#47-dispatching-server-messages)
  - [4.8 The bot commands](#48-the-bot-commands)
  - [4.9 Optional hooks for bot-mediated transfer](#49-optional-hooks-for-bot-mediated-transfer)
  - [4.10 Signals and clean shutdown](#410-signals-and-clean-shutdown)
  - [4.11 Whitelist audit for the bot](#411-whitelist-audit-for-the-bot)
- [Part 5 — Makefile: adding the bot without breaking the rules](#part-5--makefile-adding-the-bot-without-breaking-the-rules)
- [Part 6 — Testing everything](#part-6--testing-everything)
- [Part 7 — Defending it at evaluation](#part-7--defending-it-at-evaluation)
- [Appendix A — IRC message grammar](#appendix-a--irc-message-grammar)
- [Appendix B — Numeric replies reference](#appendix-b--numeric-replies-reference)
- [Appendix C — CTCP and DCC reference](#appendix-c--ctcp-and-dcc-reference)
- [Appendix D — Glossary](#appendix-d--glossary)

---

## Part 0 — What the subject actually says

### 0.1 The bonus text, verbatim

Chapter VI of `en.subject.pdf`, in full:

> **Bonus part**
>
> Here are additional features you may add to your IRC server to make it
> resemble an actual IRC server more closely:
>
> - Handle file transfer.
> - A bot.

That is the entire specification — two bullet points. There is no further
detail anywhere in the subject. This means **you** define the scope and **you**
defend it at evaluation. Both bullets are needed: the bonus is not a menu.

### 0.2 The rule that catches everyone

Immediately below those two bullets, in a red warning box:

> The bonus part will only be assessed if the mandatory part is **PERFECT**.
> Perfect means the mandatory part has been integrally done and works without
> malfunctioning. **If you have not passed ALL the mandatory requirements, your
> bonus part will not be evaluated at all.**

And from Chapter II, General rules:

> Your program should not crash in any circumstances (even when it runs out of
> memory), and should not quit unexpectedly. If it happens, your project will be
> considered non-functional and **your grade will be 0**.

Read both twice. A single crash, leak, or misbehaviour found during evaluation
means everything in this guide scores **zero** — and the "grade will be 0"
clause means it can cost you the *whole project*, not just the bonus.

This is why [Part 1](#part-1--pre-flight-fix-the-mandatory-part-first) is the
longest section of this document, and why it comes first. Your current code has
**six blocker-class defects**, one of which is a use-after-free that valgrind
will find in under ten seconds. Do not write a line of bonus code until Part 1's
checklist is green.

### 0.3 The trap: "You must not develop an IRC client"

This is the single most important thing to understand before choosing your bot
architecture, and most guides get it wrong.

Chapter IV, Mandatory Part, page 6:

> You **must not** develop an IRC client.
> You **must not** implement server-to-server communication.

A bot is, mechanically, an IRC client — it opens a TCP connection, sends
`PASS`/`NICK`/`USER`, and speaks the client side of the protocol. So does that
sentence forbid the bot the bonus explicitly asks for?

**No — and here is the argument, which you must be able to make out loud.**

1. **Scope.** That prohibition is in *Chapter IV, Mandatory Part*. It constrains
   what you submit **as the mandatory deliverable**. Chapter VI, a separate
   chapter, then explicitly asks for a bot. A subject does not forbid in
   chapter IV the thing it requires in chapter VI.
2. **Intent.** The prohibition exists because of this requirement, two pages
   later: *"Several IRC clients exist. You have to choose one of them as a
   **reference**. Your reference client will be used during the evaluation
   process."* The rule means: **do not write your own client and then use it to
   test your server**, because that lets you make both sides agree on a wrong
   protocol. You must be tested against real irssi/HexChat. It does not mean
   "no program may ever connect to your server."
3. **The whitelist proves it.** The allowed-functions list on page 6 includes
   **`connect`**. Go through the mandatory part line by line: a *server* calls
   `socket`, `setsockopt`, `bind`, `listen`, `accept`, `poll`, `recv`, `send`,
   `close`. It never calls `connect` — `connect` is what a *client* does to
   reach a server. The list also includes `gethostbyname`, `getaddrinfo` and
   `freeaddrinfo`, which a server binding `INADDR_ANY` has no use for either.
   The subject authors put outbound-connection functions on the whitelist. That
   is not an accident.

**How to actually defend this at the table:**

> "The subject forbids developing an IRC client *as the mandatory deliverable* —
> the point is that my server must be tested against a real reference client,
> which is irssi. My reference client is irssi, and that is what I'll demo the
> mandatory part with. The bonus explicitly asks for a bot, and a bot is by
> definition a program that connects to the server. `connect` and
> `getaddrinfo` are on the allowed-functions list even though the server itself
> never needs them, which tells you outbound connections were anticipated. The
> bot is a separate binary — `ircserv` contains zero bonus code, so nothing
> about the bot can affect the mandatory evaluation."

That last sentence is the strongest part. Keep the binaries separate and the
argument becomes nearly unassailable, because you can demo the entire mandatory
part with the bot deleted from disk.

**The hedge.** Some evaluators read the prohibition literally regardless. If
yours does and will not move, your fallback is a **server-side bot**: a
`Bot` object living inside `Server`, registered as a virtual client with no
socket, that `ParseCommands` routes `PRIVMSG` to. It is weaker (it proves
nothing about your network path) but it is unambiguously not "an IRC client."
Section [3.6](#36-the-fallback-a-server-side-bot) sketches it so you can pivot
in an hour if you must. Ask your evaluator early; do not discover this at
minute 40 of the defense.

### 0.4 The allowed-functions whitelist

Page 6 lists everything you are permitted to use. **The bot is bound by this
list too** — it is part of your submission.

> Everything in C++ 98.
> `socket`, `close`, `setsockopt`, `getsockname`, `getprotobyname`,
> `gethostbyname`, `getaddrinfo`, `freeaddrinfo`, `bind`, `connect`, `listen`,
> `accept`, `htons`, `htonl`, `ntohl`, `ntohs`, `inet_addr`, `inet_ntoa`,
> `inet_ntop`, `send`, `recv`, `signal`, `sigaction`, `sigemptyset`,
> `sigfillset`, `sigaddset`, `sigdelset`, `sigismember`, `lseek`, `fstat`,
> `fcntl`, `poll` (or equivalent)

Consequences you must internalise:

| Constraint | What it means for you |
|---|---|
| **`open`, `read`, `write` are NOT on the list** | For file I/O (needed if you do bot-mediated transfer), use C++98 streams — `std::ifstream` / `std::ofstream` — which fall under "Everything in C++ 98". Do **not** reach for `open`/`read`/`write` |
| **`connect` IS on the list** | Outbound connections are sanctioned. This is your bot's entry point, and your evidence in the §0.3 argument |
| **`getaddrinfo` / `gethostbyname` are on the list** | Your bot may resolve a hostname. `inet_addr("127.0.0.1")` is simpler and also allowed |
| **`fcntl` is restricted** | Page 9: *"you are allowed to use `fcntl()` only as follows: `fcntl(fd, F_SETFL, O_NONBLOCK);` Any other flag is forbidden."* Your server does exactly this in `SerSocket` and `AcceptNewClient`. Your bot must too — no `F_GETFL`, no `FD_CLOEXEC` |
| **Forking is prohibited** | Page 8: *"Forking is prohibited. All I/O operations must be non-blocking."* Your server may never `fork`, and may never spawn the bot. The bot is launched by hand, as a separate command |
| **Any external library and Boost are forbidden** | Chapter II. No libcurl, no getopt wrappers, nothing outside the C++98 standard library and the list above |

Note also, from page 7:

> Even though `poll()` is mentioned in the subject and the evaluation scale, you
> may use any equivalent such as `select()`, `kqueue()`, or `epoll()`.

You use `poll()`. Keep it — no reason to change.

### 0.5 What we are going to build

| Deliverable | Form | Where it lives |
|---|---|---|
| **File transfer** | DCC SEND relay — audited, hardened, and demoable between two irssi clients | Server (largely works already; needs proof + the Part 1 fixes) |
| **The bot** | A **separate binary**, `ircbot`, connecting to your server over TCP | New `bot/` directory, built by `make bonus` |
| **Bot commands** | `!help`, `!time`, `!ping` | `bot/` |
| **README.md** | Mandatory deliverable, currently missing entirely | Repo root — see [1.0](#10-there-is-no-readmemd-blocker) |

### 0.6 How much time this takes

| Phase | Realistic time |
|---|---|
| Part 1 — mandatory fixes | 6–10 hours (this is the real work; six blockers) |
| Part 1.0 — README.md | 45 minutes |
| Part 2 — file transfer | 1–2 hours (mostly testing and writing up the defense) |
| Parts 3–5 — the bot | 4–6 hours |
| Part 6 — testing | 2 hours |
| **Total** | **~2½ focused days** |

---

## Part 1 — Pre-flight: fix the mandatory part first

This section lists every defect I found reading the current source. Each entry
has: **what is wrong**, **the exact code**, **how an evaluator triggers it**,
**why it happens**, and **how to fix it**. Items marked **(blocker)** will cost
you the bonus — and in two cases, the whole project.

Work through them in order. After each fix, run the reproduction steps again
and confirm the behaviour changed.

---

### 1.0 There is no README.md (blocker)

#### What is wrong

Chapter V of the subject is titled **"Readme Requirements"** and is mandatory.
Your repository root contains `Makefile`, `includes/`, `srcs/`, `.gitignore` —
and no `README.md`. A missing mandatory deliverable means the mandatory part is
not "integrally done", which by §0.2 means the bonus is not assessed.

This is the cheapest blocker on the list to fix. Do it first, today.

#### Exactly what the subject demands

> A `README.md` file must be provided at the root of your Git repository. Its
> purpose is to allow anyone unfamiliar with the project (peers, staff,
> recruiters, etc.) to quickly understand what the project is about, how to run
> it, and where to find more information on the topic.
>
> The `README.md` must include at least:
>
> - The very first line must be italicized and read: *This project has been
>   created as part of the 42 curriculum by \<login1\>[, \<login2\>[, \<login3\>[...]]].*
> - A **"Description"** section that clearly presents the project, including its
>   goal and a brief overview.
> - An **"Instructions"** section containing any relevant information about
>   compilation, installation, and/or execution.
> - A **"Resources"** section listing classic references related to the topic
>   (documentation, articles, tutorials, etc.), as well as **a description of how
>   AI was used — specifying for which tasks and which parts of the project**.
> - Additional sections may be required depending on the project (e.g. usage
>   examples, feature list, technical choices, etc.).
>
> Your README must be written in English.

#### The details people lose points on

1. **The first line is literally specified.** It must be *italicised* and must
   read exactly that sentence, with your 42 login(s) substituted. In Markdown:
   `*This project has been created as part of the 42 curriculum by yrachidi.*`
   Use your real intra login, and list every team member.
2. **The section names are specified.** Use headings named exactly
   `Description`, `Instructions`, `Resources`. Not "About", not "Usage", not
   "Build".
3. **The AI disclosure is not optional.** It is inside the Resources bullet and
   it is explicit: *"a description of how AI was used — specifying for which
   tasks and which parts of the project."* Chapter III (AI Instructions) makes
   the same point twice more, and lists as a **bad practice**: *"I let Copilot
   generate my code for a key part of my project. It compiles, but I can't
   explain how it handles pipes. During the evaluation, I fail to justify and I
   fail my project."*

   Write this honestly and specifically. "AI was used to audit the mandatory
   part for protocol conformance and memory-safety defects, and to draft this
   project's bonus implementation guide (`BONUS_GUIDE.md`). All server and bot
   code was written and is understood by the authors." is a strong disclosure —
   specific about task and about part. A vague "AI helped a bit" is worse than
   useless, and a false "no AI was used" is a integrity problem.
4. **English only.** Stated explicitly.
5. **"Additional sections may be required."** For `ft_irc`, the sections worth
   adding are: a **Features** list (which commands and modes you implemented), a
   **Technical choices** section (why `poll`, why `std::map<int,Client>`, how you
   frame messages), and a **Bonus** section describing the bot and file transfer.
   These double as your defense notes.

#### Structure to write

```
*This project has been created as part of the 42 curriculum by <login>.*

# ft_irc

## Description
  What IRC is, what this server does, the goal of the project.

## Instructions
  make
  ./ircserv <port> <password>
  How to connect with the reference client (irssi), with the exact
  /connect and /join commands.

## Features
  Registration (PASS/NICK/USER), channels, operators,
  KICK / INVITE / TOPIC / MODE (i, t, k, o, l), PRIVMSG.

## Technical choices
  Single poll() loop, non-blocking fds, per-client input buffering
  for partial packets, ...

## Bonus
  The ircbot binary, and DCC file transfer support.

## Resources
  RFC 1459, RFC 2812, Modern IRC docs, ...
  How AI was used: <specific, honest disclosure>
```

#### Verify

- [ ] `README.md` exists at the repo root and is committed
- [ ] First line is italic and matches the template exactly, with real logins
- [ ] Sections `Description`, `Instructions`, `Resources` all present
- [ ] AI usage described by task and by project part
- [ ] Written in English
- [ ] Someone who has never seen the project can build and run it from the README alone — test this on a classmate

---

### 1.1 The use-after-free (blocker)

**Severity: critical. This is the single most important thing in this document.**

#### What is wrong

When a client sends `QUIT`, the server destroys that client's `Client` object,
then immediately reads from it again.

#### The code

`srcs/Server.cpp`, `Server::ParseCommands` — the tail of the function:

```cpp
	else if (cmd.command == "QUIT")
		HandleQuit(client, cmd);         // <-- destroys `client`
	...
	else if (cmd.command == "INVITE")
		HandleInvite(client, cmd);

	CheckRegistration(client);           // <-- reads the destroyed object
}
```

Follow the chain:

1. `HandleQuit` (`srcs/commands/Quit.cpp`) calls
   `DisconnectClient(client.GetFd(), reason)`.
2. `DisconnectClient` (`srcs/Server.cpp`) calls `ClearClients(fd)`.
3. `ClearClients` ends with `this->_clients.erase(fd);`.
4. `_clients` is `std::map<int, Client>`. `erase` runs `~Client()` and frees the
   node. The reference `client` — which points into that node — is now dangling.
5. Control returns to `ParseCommands`, which calls `CheckRegistration(client)`.
6. `CheckRegistration` immediately calls `client.isRegistered()`, reading freed
   memory.

#### How an evaluator triggers it

```
$ ./ircserv 6667 pass
# in another terminal
$ nc -c localhost 6667
PASS pass
NICK bob
USER bob 0 * :Bob
QUIT :bye
```

It will usually *appear* to work — freed memory often still holds the old bytes.
That is what makes it dangerous: it passes casually and fails under
`valgrind`, under `-fsanitize=address`, or at random on the evaluator's machine.
A 42 evaluator running `valgrind ./ircserv 6667 pass` and typing `QUIT` will see
an **Invalid read of size 1** and the project stops there.

#### Why it happens

`ParseCommands` takes `Client &client` and assumes the object outlives the call.
Two handlers break that assumption: `HandleQuit` always, and any path that
disconnects a client.

#### How to fix it

Pick one. Option A is the smallest change; option C is the cleanest.

**Option A — return a "client still alive" signal.**
Make the dispatch record the fd before dispatching, and re-look-up afterwards:

```
int fd = client.GetFd();
... dispatch ...
if (GetClientByFd(fd))      // still in the map?
    CheckRegistration(client);
```

Note the subtlety: you must look up by **fd**, not use the reference, because
the reference is what's dangling. `GetClientByFd` searches `_clients` by key and
returns `NULL` if the client is gone.

**Option B — make `HandleQuit` defer the actual teardown.**
Have `HandleQuit` push the fd onto a `std::vector<int> _toDisconnect` member.
`ServerRun` drains that vector at the *end* of each poll cycle, after all
`revents` have been processed. This is the standard fix for the whole class of
problem and also solves [1.2](#12-the-poll-loop-fd-skip-blocker) for free.
Strongly recommended if you have the time.

**Option C — have the dispatcher work on fds, not references.**
Change `ParseCommands(Client &client, ...)` to `ParseCommands(int fd, ...)` and
re-fetch the `Client*` at each step, bailing out the instant it returns `NULL`.
Verbose but impossible to get wrong.

#### Verify the fix

```
$ valgrind --leak-check=full --track-origins=yes ./ircserv 6667 pass
# connect, register, QUIT, then Ctrl-C
```
Zero `Invalid read` / `Invalid write`. Note that on macOS `valgrind` is often
unavailable; use `c++ -fsanitize=address -g ...` instead, or `leaks --atExit --
./ircserv 6667 pass`.

---

### 1.2 The poll-loop fd skip (blocker)

#### What is wrong

`ServerRun` iterates `_fds` by index while the handlers it calls **erase
elements from `_fds`**. Elements shift down; the loop's index does not.

#### The code

`srcs/Server.cpp`, `Server::ServerRun`:

```cpp
		for (size_t i = 0; i < this->_fds.size(); ++i)
		{
			if (this->_fds[i].revents & POLLIN)
			{
				if (this->_fds[i].fd == this->_SerSocketFd)
					AcceptNewClient();
				else
					ReceiveNewData(this->_fds[i].fd);
			}
		}
```

and `Server::ClearClients`:

```cpp
	for (size_t i = 0; i < this->_fds.size(); ++i)
	{
		if (this->_fds[i].fd == fd)
		{
			this->_fds.erase(this->_fds.begin() + i);   // <-- shifts everything after i
			break;
		}
	}
```

#### The failure

Say `_fds` is `[listen, A, B, C]` and in one `poll()` return both **A** and **C**
have data. The loop processes A at `i == 1`. A's data was `QUIT`, so
`ClearClients` erases index 1. `_fds` is now `[listen, B, C]`. The loop
increments to `i == 2`, which is now **C** — but `revents` for index 2 is B's
stale `revents` value, copied down by the erase. Depending on the values you
either:

- **skip C entirely** (its data sits unread until the next event — a hang the
  evaluator sees as "the server ignored my message"), or
- **process a stale `revents`**, calling `ReceiveNewData` on an fd that had no
  data, which blocks... except the fd is `O_NONBLOCK`, so `recv` returns `-1`
  with `EAGAIN`, which your code reads as `bytes <= 0` and **disconnects a
  perfectly healthy client**.

That last one is the nasty one. `ReceiveNewData` does:

```cpp
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
	if (bytes <= 0)
	{
		... close(fd); ClearClients(fd); ...
	}
```

`recv` returning `-1`/`EAGAIN` is **not** a disconnect — it means "no data right
now". Treating it as a disconnect is a bug in its own right (see below).

#### How an evaluator triggers it

Open three `nc` clients, register all three, have them all `PRIVMSG` a shared
channel in the same instant (paste three lines fast, or script it). Randomly,
one client drops or one message vanishes.

#### How to fix it

Three separate corrections, all needed:

1. **Never erase from `_fds` inside the loop.** Use the deferred-disconnect
   queue from [1.1 option B](#11-the-use-after-free-blocker): handlers mark fds
   for removal, `ServerRun` removes them after the `for` loop completes.
2. **Distinguish `recv() == 0` from `recv() < 0`.**
   - `0` means the peer closed the connection → disconnect.
   - `< 0` means an error. Under `O_NONBLOCK`, `errno == EAGAIN` or
     `EWOULDBLOCK` means "nothing to read" → **return without disconnecting**.
     Any other errno → disconnect.
   - Caveat for 42: the subject forbids checking `errno` after `read`/`recv` on
     some campuses' interpretation. The clean way to avoid needing `errno` at
     all is fix #1 — if you only ever call `recv` on an fd whose `POLLIN` you
     just observed *in this same cycle*, `EAGAIN` cannot happen. Confirm your
     campus's reading of the rule with your evaluator.
3. **Handle `POLLHUP` and `POLLERR`.** Right now you only test `POLLIN`. A peer
   that resets the connection sets `POLLHUP`/`POLLERR` and possibly not
   `POLLIN`, leaving a dead fd in the set forever.

---

### 1.3 Silent disconnects never reach the channel (blocker)

#### What is wrong

There are two disconnect paths and only one of them tells anybody.

| Path | Trigger | Broadcasts `QUIT` to channels? |
|---|---|---|
| `DisconnectClient` | client sent `QUIT` | ✅ yes |
| `ReceiveNewData`, `bytes <= 0` | client's socket died (Ctrl-C, network drop, `nc` closed) | ❌ **no** |

#### The code

`srcs/Server.cpp`, `Server::ReceiveNewData`:

```cpp
	if (bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		close(fd);
		ClearClients(fd);
		return;
	}
```

It closes and cleans up, but never sends `:nick!user@host QUIT :reason` to the
other members of the channels the client was in.

#### How an evaluator triggers it

```
# terminal 1
$ nc -c localhost 6667
PASS pass
NICK alice
USER alice 0 * :Alice
JOIN #test

# terminal 2 — irssi or nc, join #test too, then in terminal 1:
Ctrl-C
```

In irssi, alice never leaves `#test`. Her name stays in the user list forever.
The evaluator will absolutely test this — "kill a client without QUIT" is on
every ft_irc checklist.

#### How to fix it

Route **both** paths through `DisconnectClient`. It already does the right
thing: builds the `QUIT` message, broadcasts to shared channels, sends
`ERROR :Closing Link:`, closes, and cleans up. In `ReceiveNewData`, replace the
manual `close` + `ClearClients` with:

```
DisconnectClient(fd, "Connection reset by peer");
```

Two details:
- `DisconnectClient` calls `SendReply(fd, errClosing)` on a socket that may
  already be dead. That is harmless (`SIGPIPE` is ignored in `main.cpp`, so
  `send` just returns `-1`), but you can skip it on this path if you prefer.
- Do not call `close(fd)` twice. `DisconnectClient` already closes.

---

### 1.4 Unregistered clients can run any command (blocker)

#### What is wrong

Only some handlers check `client.isRegistered()`. The rest do not.

| Handler | File | Checks registration? |
|---|---|---|
| `HandlePrivmsg` | `srcs/commands/Privmsg.cpp` | ✅ |
| `HandleTopic` | `srcs/commands/Topic.cpp` | ✅ |
| `HandlePart` | `srcs/commands/Part.cpp` | ✅ |
| `HandleJoin` | `srcs/commands/Join.cpp` | ❌ **missing** |
| `HandleKick` | `srcs/commands/Kick.cpp` | ❌ **missing** |
| `HandleMode` | `srcs/commands/Mode.cpp` | ❌ **missing** |
| `HandleInvite` | `srcs/commands/Invite.cpp` | ❌ **missing** |

#### How an evaluator triggers it

```
$ nc -c localhost 6667
JOIN #hack
```

No `PASS`. No `NICK`. No `USER`. The channel is created and this anonymous
connection is made its **operator**. The password — the entire point of the
`<password>` argument in the subject — is bypassed.

This is the classic ft_irc evaluation question: *"Can I do anything before I
authenticate?"* The answer must be no.

#### How to fix it

Add the same guard the other handlers use, at the top of each of the four
handlers:

```
if (!client.isRegistered())
{
    SendReply(client.GetFd(), ERR_NOTREGISTERED(client.getNick().empty() ? "*" : client.getNick()));
    return;
}
```

**Better:** enforce it once, centrally, in `Server::ParseCommands`. Before the
dispatch chain, allow only the pre-registration commands and reject everything
else:

```
if (!client.isRegistered()
    && cmd.command != "PASS" && cmd.command != "NICK"
    && cmd.command != "USER" && cmd.command != "CAP"
    && cmd.command != "QUIT" && cmd.command != "PING")
{
    SendReply(...ERR_NOTREGISTERED...);
    return;
}
```

Central enforcement means a future command cannot forget the check. Note
`PING` is in the allow-list because some clients ping during the handshake, and
`QUIT` because a client must always be able to leave.

**Also note the ordering rule:** `PASS` must be accepted *before* registration
completes, and re-sending it after registration must produce `462`
(`ERR_ALREADYREGISTERED`). `HandlePass` already gets this right.

---

### 1.5 Malformed numeric replies

#### What is wrong

Three handlers emit numeric replies that are missing both the **server prefix**
and the **target nickname**. Real IRC clients silently discard these.

#### The code

`srcs/commands/Join.cpp`:
```cpp
SendReply(client.GetFd(), "461 JOIN :Not enough parameters");
SendReply(client.GetFd(), "403 " + ChannelName + " :No such channel");
SendReply(client.GetFd(), "331 " + client.getNick() + " " + ChannelName + " :No topic is set");
```
`srcs/commands/Kick.cpp`:
```cpp
SendReply(client.GetFd(), "461 KICK :Not enough parameters");
SendReply(client.GetFd(), "403 " + channelName + " :No such channel");
SendReply(client.GetFd(), "482 " + channelName + " :You're not channel operator");
SendReply(client.GetFd(), "401 " + targetNick + " :No such nick");
SendReply(client.GetFd(), "441 " + targetNick + " " + channelName + " :They aren't on that channel");
```
`srcs/commands/Invite.cpp`: same pattern for `461`, `403`, `442`, `482`, `401`,
`443`, `341`.

#### Why it is wrong

The IRC message grammar for a numeric reply is:

```
:<server> <3-digit numeric> <target-nick> <params...> :<trailing>
 ^^^^^^^^                   ^^^^^^^^^^^^
 required                   required
```

`"461 JOIN :Not enough parameters"` is missing the leading `:ft_ircserv` and
puts `JOIN` where the client's nickname must be. A client parsing this reads
the target as `JOIN`, decides the message is not addressed to it, and drops it.

The correct form is:
```
:ft_ircserv 461 bob JOIN :Not enough parameters
```

#### The irony

You **already have** correct builders for all of these in
`includes/Replies.hpp` — `ERR_NEEDMOREPARAMS`, `ERR_NOSUCHCHANNEL`,
`ERR_CHANOPRIVSNEEDED`, `ERR_NOSUCHNICK`, `ERR_USERNOTINCHANNEL`,
`RPL_NOTOPIC`, `RPL_TOPIC`, `RPL_NAMREPLY`, `RPL_ENDOFNAMES`, `RPL_INVITING`,
`ERR_USERONCHANNEL`, `ERR_NOTONCHANNEL`. `Privmsg.cpp` and `Topic.cpp` use them
correctly. `Join.cpp`, `Kick.cpp` and `Invite.cpp` were written before (or
independently of) `Replies.hpp` and hand-roll the strings instead.

#### How to fix it

Replace every hand-rolled numeric in those three files with the matching
`Replies.hpp` helper. Mechanical, low-risk, and it makes the inconsistency
disappear. Grep for the pattern to find them all:

```
grep -rn 'SendReply(.*"[0-9][0-9][0-9] ' srcs/
```

Anything that shows up did not use a helper.

---

### 1.6 Malformed JOIN echo

#### What is wrong

`srcs/commands/Join.cpp` announces a join as:

```cpp
std::string joinMsg = ":" + client.getNick() + " JOIN :" + ChannelName;
```

That produces `:bob JOIN :#test`. The prefix must be the **full user prefix**,
`nick!user@host`:

```
:bob!bob@127.0.0.1 JOIN #test
```

#### Why it matters

Clients use the `user@host` part to build their internal user list and to match
the join against their own nick. irssi with a bare-nick prefix may show the join
but fail to associate it with a hostmask, which then breaks `KICK`/`MODE`
targeting in some clients. `HexChat` is stricter and may ignore it.

Note that `Part.cpp`, `Topic.cpp`, `Privmsg.cpp` and `Mode.cpp` all correctly
use `client.prefix()`, which is defined in `srcs/Client.cpp` as:

```cpp
return this->_nick + "!" + this->_user + "@" + (this->_host.empty() ? this->_ipAdd : this->_host);
```

`Join.cpp` and `Kick.cpp` do not.

#### How to fix it

```
std::string joinMsg = ":" + client.prefix() + " JOIN " + ChannelName;
```

Two changes: `client.prefix()` instead of `client.getNick()`, and drop the `:`
before the channel name (the channel is a normal middle param, not a trailing
param — both are technically accepted, but the non-trailing form is standard).

Do the same in `Kick.cpp`:
```
std::string kickMsg = ":" + client.prefix() + " KICK " + channelName + " " + targetNick + " :" + comment;
```

---

### 1.7 No comma-separated channel list in JOIN

#### What is wrong

The RFC allows `JOIN #a,#b,#c` and `JOIN #a,#b key1,key2`. `HandleJoin` reads
only `cmd.params[0]` as a single channel name.

#### How an evaluator triggers it

In irssi: `/join #one,#two`. Your server creates a single channel literally
named `#one,#two`.

#### Why this one matters more than it looks

`HandlePart` and `HandlePrivmsg` **already** split on commas — look at the
`while (start < channels.size())` loops in `srcs/commands/Part.cpp` and
`srcs/commands/Privmsg.cpp`. So your server can `PART` two channels at once but
cannot `JOIN` them. An evaluator who spots the inconsistency will test it.

#### How to fix it

Copy the comma-splitting loop from `Part.cpp` into `Join.cpp` and wrap the
existing body in it. You additionally need to split the **keys** parameter
(`cmd.params[1]`) on commas and pair key `i` with channel `i`.

Also fix, while you are in there:
- **Accept `&` as a channel prefix.** `Join.cpp` tests `ChannelName[0] != '#'`,
  but `Mode.cpp` and `Privmsg.cpp` both accept `#` *and* `&`. Be consistent —
  either accept both everywhere or reject `&` everywhere.
- **Validate the rest of the channel name.** A channel name may not contain a
  space, a comma, a `\a` (0x07), or a `:`. Length cap is 200. Reject with `403`.

---

### 1.8 Unbounded input buffer

#### What is wrong

`Client::AppendToBuffer` appends every byte received and only ever shrinks when
a `\n` is found. A client that sends data without a newline grows `_buffer`
forever.

#### The code

`srcs/Server.cpp`, `ReceiveNewData`, and `srcs/Client.cpp`:
```cpp
void Client::AppendToBuffer(const std::string& data) { this->_buffer += data; }
```

#### How an evaluator triggers it

```
$ yes | tr -d '\n' | nc localhost 6667
```

Watch the server's memory in `top`. It climbs until the process is killed. This
is a denial of service — one unauthenticated client kills the server.

#### How to fix it

RFC 1459 §2.3 caps an IRC message at **512 bytes including the trailing
`\r\n`**. Enforce it:

- After appending, if `_buffer.size() > 512` and it contains no `\n`, the client
  is sending a malformed oversized line. Either disconnect it
  (`ERROR :Request too long`) or discard up to the next newline.
- Also cap the number of buffered lines if you want belt-and-braces.

Pick one policy, implement it, and be ready to explain it. "I cap at 512 per
RFC 1459 and drop the connection" is a perfectly good answer.

---

### 1.9 Sending without POLLOUT (blocker — explicit grade-0 clause)

**This is a blocker, and the subject spells out the penalty.**

#### The subject text

Page 8, in a red warning box:

> Because you have to use non-blocking file descriptors, it is possible to use
> read/recv or write/send functions with no `poll()` (or equivalent), and your
> server wouldn't be blocking. However, it would consume more system resources.
> **Therefore, if you attempt to read/recv or write/send in any file descriptor
> without using `poll()` (or equivalent), your grade will be 0.**

And immediately above it:

> Only **1** `poll()` (or equivalent) can be used for handling all these
> operations (**read, write**, but also listen, and so forth).

Note the parenthetical: *read, **write***. `poll()` must gate your writes, not
only your reads.

#### What is wrong

Both of your output paths call `send()` with no `POLLOUT` involved anywhere. You
never set `POLLOUT` in `events`, and you never test it in `revents`.

`srcs/Server.cpp`:
```cpp
void Server::SendReply(int fd, const std::string &message)
{
	std::string msg = message;
	if (msg.size() < 2 || msg.substr(msg.size() - 2) != "\r\n")
		msg += "\r\n";
	send(fd, msg.c_str(), msg.size(), 0);      // <-- no poll gate, return value discarded
}
```

`srcs/Client.cpp`:
```cpp
void Client::queueOutput(const std::string& msg)
{
	std::string output = msg;
	if (output.size() < 2 || output.substr(output.size() - 2) != "\r\n")
		output += "\r\n";
	send(this->_fd, output.c_str(), output.size(), 0);   // <-- same
}
```

Note the method name: `queueOutput` does not queue anything. It sends
immediately. Every `Channel::broadcast` goes through it.

#### Two distinct problems

**Problem 1 — the compliance problem.** You send on fds whose `POLLOUT` you
never asked `poll()` about. An evaluator reading the red box and then reading
`SendReply` has a defensible case for grade 0. Whether a given evaluator
enforces it this strictly varies, but you cannot rely on leniency, and you
cannot argue your way out of a box that says *"your grade will be 0"*.

**Problem 2 — the correctness problem.** Every client fd is `O_NONBLOCK` (set in
`AcceptNewClient`). On a non-blocking socket, `send()` is allowed to write
**fewer bytes than requested** and return that shorter count, or return `-1`
with `EAGAIN` when the kernel's send buffer is full. Your code assumes it always
writes everything and discards the return value. When it doesn't, those bytes
are **gone** — and because the client frames on `\r\n` exactly like you do, a
truncated line corrupts its stream from that point onward.

#### How an evaluator triggers problem 2

Hard to hit by accident on loopback; easy on purpose:

```
# client A: connect, register, JOIN #test, then suspend it so it stops reading
$ nc -C localhost 6667
... register, JOIN #test ...
Ctrl-Z

# client B: flood #test
$ while true; do echo "PRIVMSG #test :flood"; done | nc -C localhost 6667
```

A's kernel receive buffer fills, so the server's `send()` to A starts returning
short counts. Those messages vanish. `fg` client A and its stream is corrupt.

Even if they never run this, **"what happens when `send()` returns less than you
asked for?"** is one of the most common ft_irc defense questions. "Nothing, I
lose the data" fails.

#### How to fix it — the real fix

Both problems have the same solution: a genuine output queue driven by
`POLLOUT`.

1. Give `Client` a `std::string _outBuffer` (this is what the field was always
   meant to be — the method is already named `queueOutput`).
2. `queueOutput` and `SendReply` **append** to `_outBuffer`. They stop calling
   `send()` entirely.
3. In `ServerRun`, each cycle, before `poll()`: for every client fd, set
   `events = POLLIN`, and OR in `POLLOUT` **only if** that client's `_outBuffer`
   is non-empty.
4. After `poll()` returns, for each fd with `revents & POLLOUT`: call `send()`
   once with the whole `_outBuffer`, take the returned count `n`, and
   `_outBuffer.erase(0, n)`. Do not loop until empty — let the next `poll()`
   cycle handle the remainder. That is the whole point.
5. When `_outBuffer` becomes empty, **stop requesting `POLLOUT`** for that fd.

**Step 5 is not optional and it is where everyone breaks.** A socket that is
writable is *always* writable, so if you leave `POLLOUT` set with nothing to
send, `poll()` returns instantly, forever, and your server spins at 100% CPU.
An evaluator running `top` will see it. Since you rebuild `events` from
`_outBuffer.empty()` every cycle in step 3, you get this for free — which is
exactly why step 3 is written that way.

#### Ordering detail that matters

If a client is disconnecting (`QUIT`, or `ERROR :Closing Link:`), its final
message is sitting in `_outBuffer` unsent. If you `close()` the fd immediately,
the client never receives it. Handle this by:

- flushing that client's `_outBuffer` with a final direct `send()` before
  `close()` — acceptable, and what most students do; or
- marking the client "closing", refusing new input, and destroying it only once
  `_outBuffer` is empty — cleaner, more work.

Either is defensible. Know which one you chose and why.

#### Verify the fix

- [ ] `grep -n 'send(' srcs/` shows `send` called from exactly one place — the `POLLOUT` handler
- [ ] `_fds[i].events` is rebuilt every cycle from `_outBuffer.empty()`
- [ ] Idle server with clients connected sits at **0% CPU** in `top`
- [ ] The suspend-and-flood test above loses no messages
- [ ] `QUIT` still delivers `ERROR :Closing Link:` before the socket closes

---

### 1.10 Smaller conformance gaps

These will not sink you on their own, but each is a question an evaluator might
ask. Fix the cheap ones.

| # | Issue | Location | Fix |
|---|---|---|---|
| a | `MODE +k` with no key parameter is silently ignored | `Mode.cpp`, the `m == 'k'` branch | Reply `ERR_NEEDMOREPARAMS` |
| b | `MODE +o <nick>` for a nonexistent nick replies `441` (`ERR_USERNOTINCHANNEL`) | `Mode.cpp`, `m == 'o'` branch | Use `401` `ERR_NOSUCHNICK` when the nick does not exist at all; keep `441` only when the nick exists but is not in the channel |
| c | `MODE +l 0` or a negative limit is ignored without a reply | `Mode.cpp`, `m == 'l'` | Either reject explicitly or document that `l <= 0` means "unset" |
| d | `MODE` on a user (`MODE bob +i`) returns silently | `Mode.cpp`, the `target[0] != '#' && != '&'` early return | Reply `ERR_UMODEUNKNOWNFLAG` (501) or `ERR_NOSUCHNICK`, or state that user modes are out of scope |
| e | Server never sends `PING` to clients; a dead-but-not-closed connection lives forever | `ServerRun` | Optional: track last-activity per client, `PING` after N seconds, disconnect after no `PONG`. Nice-to-have, not required |
| f | No `PONG` handler | `ParseCommands` | Clients send `PONG` in reply to your `PING`. Currently it falls through to "unknown command" (which silently does nothing). Add a no-op `PONG` branch |
| g | Unknown commands get no reply | `ParseCommands` | Add a final `else` sending `ERR_UNKNOWNCOMMAND` (421). Currently `LIST`, `WHO`, `NAMES`, `WHOIS` etc. produce total silence, which looks like a hang |
| h | `NICK` collision during registration | `Nick.cpp` | Correct already — it checks `GetClientByNick`. Good |
| i | `Channel` default `_topicRestricted = true` | `Channel.cpp` constructors | This is a deliberate choice (`+t` by default matches most real servers). Just be ready to say so — an evaluator reading `MODE #chan` output will see `+t` on a fresh channel and ask why |
| j | `Server` copy-assignment copies `_clients` and `_channels` | `Server.cpp` `operator=` | The `Client*` pointers inside the copied `_channels` would point into the *original* `_clients` map. Harmless today because nothing copies a `Server`, but it is a landmine. Consider making `Server` non-copyable (private, undefined copy ctor and `operator=` — the C++98 idiom) |
| k | `_buffer` is not cleared on partial-line disconnect | `ClearClients` | The whole `Client` is erased, so this is fine. No action |

#### On (g) — unknown commands

This one is worth doing. Add to the end of the `if/else if` chain in
`ParseCommands`:

```
else
    SendReply(client.GetFd(), ":ft_ircserv 421 " + nick + " " + cmd.command + " :Unknown command");
```

Without it, an evaluator typing `/list` in irssi sees nothing happen and
concludes the server hung.

---

### 1.11 Subject-compliance audit

Separate from bugs: these are places where your code may violate a stated rule.
Check each one against your source before the defense.

| Rule (subject page) | Your status | Action |
|---|---|---|
| "Only 1 `poll()` can be used for handling all these operations (read, **write**, but also listen)" (p8) | ❌ writes bypass `poll` | See [1.9](#19-sending-without-pollout-blocker--explicit-grade-0-clause) — **blocker** |
| "if you attempt to read/recv or write/send in any file descriptor without using `poll()`, your grade will be 0" (p8) | ❌ same | See [1.9](#19-sending-without-pollout-blocker--explicit-grade-0-clause) |
| "Forking is prohibited" (p8) | ✅ no `fork` anywhere | Keep it that way — the bot must **not** be spawned by the server |
| "All I/O operations must be non-blocking" (p8) | ⚠️ partially | Reads are fine (`O_NONBLOCK` + `poll`); writes are not until 1.9 is done |
| "you are allowed to use `fcntl()` only as follows: `fcntl(fd, F_SETFL, O_NONBLOCK);`" (p9) | ✅ correct in `SerSocket` and `AcceptNewClient` | The bot must obey this too |
| "Any external library and Boost are forbidden" (p3) | ✅ | Applies to the bot as well |
| "C++ 98 standard... should still compile if you add `-std=c++98`" (p3) | ✅ the Makefile already passes `-std=c++98` | Keep every bonus file C++98 |
| "compile with `c++` using the flags `-Wall -Wextra -Werror`" (p3) | ✅ | Bonus sources must build clean under the same flags |
| "Makefile... must not perform unnecessary relinking" (p3) | ⚠️ untested | Run `make` twice; the second must print nothing but "up to date"-style output. See [Part 5](#part-5--makefile-adding-the-bot-without-breaking-the-rules) |
| "Your `Makefile` must at least contain the rules: `$(NAME)`, `all`, `clean`, `fclean` and `re`" (p3) | ✅ all five present | Note: **`bonus` is not a required rule** in v10.0, unlike some other 42 subjects |
| "You must not develop an IRC client" (p6) | ⚠️ interpretation | See [0.3](#03-the-trap-you-must-not-develop-an-irc-client). Have the argument ready |
| "You must not implement server-to-server communication" (p6) | ✅ | Do not add it as a bonus — it is explicitly forbidden, not rewarded |
| "Files to Submit: `Makefile`, `*.{h, hpp}`, `*.cpp`, `*.tpp`, `*.ipp`, an optional configuration file" (p6) | ✅ | Bot sources are `.cpp`/`.hpp`, so they fit. Do not commit `.o` files or the binaries — your `.gitignore` already excludes them |
| `README.md` required (p10) | ❌ **missing** | See [1.0](#10-there-is-no-readmemd-blocker) — **blocker** |
| "Your reference client will be used during the evaluation process" (p8, p12) | ⚠️ undeclared | Pick one — irssi is the usual choice — install it, test with it, and say so in the README |

#### The subject's own test — run it exactly

Page 9, section IV.3 "Test example":

> Verify every possible error and issue, such as receiving partial data, low
> bandwidth, etc.
>
> To ensure that your server correctly processes all data sent to it, the
> following simple test using `nc` can be performed:
>
> ```
> \$> nc -C 127.0.0.1 6667
> com^Dman^Dd
> \$>
> ```
>
> Use **ctrl+D** to send the command in several parts: `'com'`, then `'man'`,
> then `'d\n'`.
>
> In order to process a command, you have to first aggregate the received
> packets in order to rebuild it.

This is a **named, guaranteed** evaluation test. Run it yourself:

```
$ nc -C 127.0.0.1 6667
```
then type `PASS`, Ctrl-D, ` pas`, Ctrl-D, `s`, Enter. The server must treat it
as the single command `PASS pass`.

Your framing logic in `ReceiveNewData` already handles this correctly — it
accumulates into `Client::_buffer` and only dispatches on `\n`. Good. But
**test it and watch it work**, because you will be asked to demonstrate it, and
you want to have seen it pass before the evaluator does.

Note the `-C` flag: it makes `nc` send `\r\n` line endings, matching what a real
IRC client sends. Test both with and without it — your `ReceiveNewData` strips a
trailing `\r` if present, so both must work.

---

### 1.12 Pre-flight checklist

Do not start Part 2 until every box is ticked.

**Deliverables**
- [ ] `README.md` exists, follows Chapter V exactly, includes the AI disclosure (1.0)
- [ ] Reference client chosen, installed, and named in the README
- [ ] `make` twice in a row does not relink (subject p3)
- [ ] `make fclean && make` builds clean under `-Wall -Wextra -Werror -std=c++98`
- [ ] No `.o` files or binaries committed

**Crash safety** — "your grade will be 0" territory
- [ ] `QUIT` does not touch freed memory (1.1) — verified under ASan or valgrind
- [ ] Two clients disconnecting in the same poll cycle both clean up correctly (1.2)
- [ ] `_fds` is never mutated inside the `ServerRun` loop (1.2)
- [ ] `POLLHUP` / `POLLERR` are handled (1.2)
- [ ] Server survives `yes | tr -d '\n' | nc localhost 6667` (1.8)
- [ ] Ctrl-C on the server exits cleanly, closing every fd
- [ ] Zero memory leaks at exit

**Subject compliance**
- [ ] Every `send()` is gated by `POLLOUT` from the single `poll()` (1.9) — **grade-0 clause**
- [ ] Idle server with clients connected sits at 0% CPU (1.9, step 5)
- [ ] `send()` short writes lose no data (1.9)
- [ ] No `fork` anywhere
- [ ] `fcntl` used only as `fcntl(fd, F_SETFL, O_NONBLOCK)`
- [ ] No external libraries

**Protocol correctness**
- [ ] Nothing works before `PASS` + `NICK` + `USER` (1.4)
- [ ] Wrong password is rejected with `464` and the client cannot proceed
- [ ] Every numeric reply has the `:ft_ircserv` prefix and the target nick (1.5)
- [ ] `JOIN` echo uses the full `nick!user@host` prefix (1.6)
- [ ] `JOIN #a,#b` joins two channels (1.7)
- [ ] Unknown commands reply `421` (1.10g)
- [ ] `PONG` is accepted without complaint (1.10f)

**Behaviour under the reference client**
- [ ] irssi connects, registers, joins, and shows the correct user list
- [ ] A `kill -9` on a client removes it from every channel, and other members see the `QUIT` (1.3)
- [ ] Two irssi clients can hold a `PRIVMSG` conversation, in-channel and direct
- [ ] All five operator modes work: `+i`, `+t`, `+k`, `+o`, `+l` — and their `-` forms
- [ ] `KICK`, `INVITE`, `TOPIC` all behave per the subject

**Partial-data handling — the subject's own named test**
- [ ] `nc -C 127.0.0.1 6667` with `com`^D`man`^D`d\n` is rebuilt into one command (1.11)
- [ ] A message split across two TCP packets is reassembled:
      `printf 'PRIV'; sleep 1; printf 'MSG #a :hi\r\n'` on one connection
- [ ] Partial command followed by an abrupt disconnect does not crash

When this list is green, your mandatory part is defensible and the bonus is
worth building. **Commit here. Tag it.** You want a known-good state to return
to when the bonus work inevitably breaks something.

---

## Part 2 — File transfer (DCC)

> Subject, Chapter VI: **"Handle file transfer."** Four words. This part explains
> what those four words mean in the IRC protocol, what your server's actual job
> is, why most of the work is already done in your code, and how to prove it at
> the defense.

### 2.1 What "handle file transfer" means

Here is the thing that surprises almost everyone: **in IRC, the server does not
transfer files.** There is no `SENDFILE` command in RFC 1459 or RFC 2812. There
never was.

File transfer on IRC is done by **DCC** — *Direct Client-to-Client* — a
convention layered on top of IRC. The two clients open a **separate, direct TCP
connection to each other**, completely bypassing the server. The file bytes never
touch your `ircserv` process.

So what does the server do? It does exactly one thing: **it relays the
handshake**. Client A tells client B "connect to me at this IP and port to get
this file", and that message travels as an ordinary `PRIVMSG` through your
server. Once B has that message, A and B talk directly.

```
                    ┌──────────────┐
     ①  PRIVMSG     │   ircserv    │   ②  PRIVMSG
     (DCC offer)    │  (your code) │   (DCC offer)
   ┌───────────────▶│              │───────────────┐
   │                └──────────────┘               ▼
┌──┴───────┐                                  ┌──────────┐
│ Client A │                                  │ Client B │
│ (sender) │                                  │(receiver)│
└──────────┘                                  └──────────┘
      ▲                                             │
      │      ③  direct TCP connection               │
      │         the file bytes flow here            │
      └─────────────────────────────────────────────┘
                 your server sees NONE of this
```

**Your server's job is steps ① and ②, and nothing else.**

This is genuinely how real IRC works — it is not a shortcut or a cop-out. When
you defend this, say it plainly and confidently:

> "IRC has no file-transfer command. File transfer is DCC, which is
> client-to-client: the two clients open a direct TCP socket and the server's
> only role is to relay the CTCP handshake inside a PRIVMSG. My server relays it
> byte-for-byte without mangling it, which is exactly what an IRC server is
> supposed to do. Here are two irssi clients transferring a file through my
> server."

An evaluator who expects your server to store and forward the file bytes has
misunderstood IRC. Be ready to explain — politely — with the diagram above.

### 2.2 How DCC SEND actually works

Step by step, for `alice` sending `report.pdf` to `bob`:

1. **Alice's client opens a listening socket.** It calls `socket`, `bind` on an
   ephemeral port, and `listen`. Say the kernel gives it port `52341`. Alice's
   client also determines its own IP, say `127.0.0.1`.

2. **Alice's client sends a CTCP DCC SEND offer** to bob, as a normal `PRIVMSG`:

   ```
   PRIVMSG bob :\x01DCC SEND report.pdf 2130706433 52341 184320\x01
   ```

   The `\x01` bytes (ASCII 0x01, `SOH`) mark this as **CTCP** — Client-To-Client
   Protocol — an in-band convention for structured messages that clients
   interpret rather than display. The same mechanism carries `\x01ACTION waves\x01`
   (the `/me` command) and `\x01VERSION\x01`.

3. **Your server relays it.** `HandlePrivmsg` looks up `bob`, builds
   `:alice!alice@127.0.0.1 PRIVMSG bob :\x01DCC SEND report.pdf 2130706433 52341 184320\x01`,
   and sends it. **The server does not know or care what CTCP is.** To your code
   this is an opaque text payload. That is correct behaviour.

4. **Bob's client parses the offer** and shows "alice offers report.pdf
   (180 KB) — accept?"

5. **Bob accepts.** His client calls `connect` to `127.0.0.1:52341` — a direct
   TCP connection to Alice's listening socket, with your server not involved.

6. **Alice's client `send`s the file bytes** down that socket. Bob's client
   `recv`s them and writes the file to disk.

7. **Bob's client acknowledges** by sending back the running byte count as a
   4-byte big-endian integer after each chunk (this is the quirky part of the
   DCC spec, and it is why DCC is slow — but again, not your problem).

Steps 1, 2, and 4–7 belong to the clients. **Only step 3 is yours.**

### 2.3 The exact wire format

Learn this by heart; you will be asked to read one live off the terminal.

```
PRIVMSG <target> :\x01DCC SEND <filename> <ip> <port> <filesize>\x01
                  ▲                                              ▲
                  └── 0x01                                  0x01 ┘
```

| Field | Meaning | Example |
|---|---|---|
| `DCC` | the CTCP tag | `DCC` |
| `SEND` | the DCC subcommand | `SEND`, `CHAT`, `RESUME`, `ACCEPT` |
| `<filename>` | base name only, no path. If it contains spaces it is wrapped in double quotes | `report.pdf` or `"my report.pdf"` |
| `<ip>` | **the sender's IPv4 as a 32-bit unsigned integer in host byte order, printed in decimal** | `2130706433` |
| `<port>` | the sender's listening TCP port | `52341` |
| `<filesize>` | size in bytes, decimal. Optional in the original spec, universally sent now | `184320` |

#### The IP encoding — the detail everyone stumbles on

`2130706433` is not a typo and not a mistake. DCC predates dotted-quad
conventions in protocol text and encodes the IPv4 address as a **single decimal
integer**.

To convert `127.0.0.1`:

```
127.0.0.1  ->  127 * 256³  +  0 * 256²  +  0 * 256  +  1
           ->  127 * 16777216 + 0 + 0 + 1
           ->  2130706432 + 1
           ->  2130706433
```

And back:
```
2130706433 / 16777216       = 127   remainder 1
          1 / 65536         = 0     remainder 1
          1 / 256           = 0     remainder 1
          1                 = 1
                            -> 127.0.0.1
```

Useful one-liners for testing:

```
# dotted-quad -> DCC integer
$ python3 -c "import ipaddress;print(int(ipaddress.IPv4Address('127.0.0.1')))"
2130706433

# DCC integer -> dotted-quad
$ python3 -c "import ipaddress;print(ipaddress.IPv4Address(2130706433))"
127.0.0.1
```

If you prefer to stay in the shell:
```
$ printf '%d\n' $((127*16777216 + 0*65536 + 0*256 + 1))
2130706433
```

**Why this matters to you even though your server doesn't parse it:** if you
ever add DCC-aware logging (§2.7), you will decode this field, and an evaluator
may ask "why is the IP a giant number?" Knowing the answer instantly is a
credibility win. `htonl`/`ntohl` are on the allowed-functions list precisely
because of conversions like this one.

#### The other DCC subcommands

| Subcommand | Format | Purpose |
|---|---|---|
| `DCC SEND` | `DCC SEND <file> <ip> <port> <size>` | Offer a file |
| `DCC CHAT` | `DCC CHAT chat <ip> <port>` | Direct chat, server bypassed |
| `DCC RESUME` | `DCC RESUME <file> <port> <position>` | Receiver asks to resume from a byte offset |
| `DCC ACCEPT` | `DCC ACCEPT <file> <port> <position>` | Sender confirms the resume |

All four are just `PRIVMSG` payloads. Your server relays all of them
identically, with zero special-casing. Mention `RESUME`/`ACCEPT` at the defense
to show you read the spec rather than the first blog post.

### 2.4 What your server must not do

Since your only job is relaying, the failure modes are all forms of *damaging
the payload in transit*. Audit for each:

| Must not | Why it breaks DCC |
|---|---|
| **Strip or alter `\x01` bytes** | The CTCP markers are the entire signal. Without them the receiving client renders `DCC SEND report.pdf 2130706433 ...` as literal chat text |
| **Trim or collapse whitespace inside the trailing param** | The DCC fields are space-separated. Collapsing runs of spaces corrupts a filename that contains them |
| **Truncate the message** | A 512-byte IRC line easily fits a DCC offer, but a truncated one is unparseable. This is exactly what the missing send-queue in [1.9](#19-sending-without-pollout-blocker--explicit-grade-0-clause) causes |
| **Re-encode or filter non-ASCII** | Filenames may be UTF-8. Pass bytes through unchanged |
| **Reject the message as "invalid"** | Do not validate `PRIVMSG` payload content. It is opaque |
| **Split on `:` inside the trailing param** | A Windows path or a URL in a filename contains `:`. Only the *first* `:` starts the trailing param |

That last one is worth dwelling on, because it is a parser bug that only ever
shows up under DCC.

### 2.5 Auditing your PRIVMSG path for DCC-safety

Walk the payload through your actual code and confirm nothing damages it.

#### Stage 1 — framing, `Server::ReceiveNewData`

```cpp
size_t pos = c->GetBuffer().find('\n');
if (pos == std::string::npos) break;

std::string line = c->GetBuffer().substr(0, pos);
if (!line.empty() && line[line.size() - 1] == '\r')
	line.erase(line.size() - 1);
c->GetBuffer().erase(0, pos + 1);
```

**Verdict: safe.** It splits on `\n` and strips one trailing `\r`. `\x01` is
untouched — it is neither `\r` nor `\n`. Binary-ish bytes inside the line
survive because `std::string` is byte-transparent, not NUL-terminated-C-string
semantics. Good.

One caveat: a filename containing a literal newline would break framing — but
that is true of every IRC server, and no client will send one.

#### Stage 2 — parsing, `ParseLine` in `srcs/Command.cpp`

```cpp
while (cmd.params.size() < 15)
{
	i = skipWhiteSpaces(line, i);
	if (i >= line.size())
		break;
	if (line[i] == ':')
	{
		cmd.params.push_back(line.substr(i + 1));   // <-- rest of line, verbatim
		break;
	}
	cmd.params.push_back(readToSpace(line, i));
}
```

**Verdict: safe, and this is the important line.** When the parser hits the `:`
that starts the trailing parameter, it takes `substr(i + 1)` — **everything to
the end of the line, in one piece, unmodified**, and then `break`s. It does not
tokenise the trailing param, does not split on further `:`, does not touch
`\x01`. This is exactly the RFC 1459 rule and exactly what DCC needs.

So for the input:
```
PRIVMSG bob :\x01DCC SEND report.pdf 2130706433 52341 184320\x01
```
you get `params[0] == "bob"` and
`params[1] == "\x01DCC SEND report.pdf 2130706433 52341 184320\x01"` — one
param, intact, `\x01` on both ends. Correct.

#### Stage 3 — handling, `srcs/commands/Privmsg.cpp`

```cpp
std::string targets = cmd.params[0];
std::string text = cmd.params[1];
for (size_t i = 2; i < cmd.params.size(); ++i)
	text += " " + cmd.params[i];
...
std::string formattedMsg = ":" + client.prefix() + " PRIVMSG " + target + " :" + text;
```

**Verdict: safe.** `text` is `params[1]` verbatim. The `for` loop that re-joins
`params[2..]` never runs for a trailing param, because the parser `break`s after
pushing the trailing — so `params.size()` is exactly 2. The payload is then
re-emitted after `" :"`, unchanged.

**Conclusion: your PRIVMSG path is already DCC-clean.** This is not luck — it
follows from the parser implementing the trailing-parameter rule correctly. Say
exactly that at the defense.

#### Stage 4 — output, `SendReply` / `queueOutput`

```cpp
if (msg.size() < 2 || msg.substr(msg.size() - 2) != "\r\n")
	msg += "\r\n";
send(fd, msg.c_str(), msg.size(), 0);
```

**Verdict: two problems, both from [1.9](#19-sending-without-pollout-blocker--explicit-grade-0-clause).**

1. `send`'s return value is discarded, so a short write **truncates the DCC
   offer**. A truncated offer is an unparseable offer — the transfer silently
   never starts. This is the one place where the 1.9 bug is directly visible in
   the file-transfer demo.
2. `msg.c_str()` is fine here — `send` is given `msg.size()`, not `strlen`, so
   embedded `\x01` (and even embedded NUL) transit correctly. No bug, but know
   why: if this had used `strlen(msg.c_str())` it would truncate at the first
   NUL byte.

**Fix 1.9 before demoing file transfer.** The two are linked.

#### The audit, as a checklist

- [ ] Framing splits only on `\n`, strips only one trailing `\r` (`ReceiveNewData`)
- [ ] Trailing param is taken with `substr(i + 1)` in one piece (`ParseLine`)
- [ ] `PRIVMSG` re-emits `params[1]` unmodified (`Privmsg.cpp`)
- [ ] Output uses `msg.size()`, never `strlen` (`SendReply`, `queueOutput`)
- [ ] Output cannot truncate on a short `send` (**needs 1.9**)
- [ ] No code anywhere inspects, validates, or rewrites `PRIVMSG` payload content
- [ ] `grep -n "0x01\|\\\\001\|\\\\x01" srcs/ includes/` returns nothing — you special-case CTCP nowhere, which is correct

### 2.6 Making it demoable: the /dcc test

Relaying correctly is worth nothing at the defense if you cannot **show** it.
Here is the exact procedure. Rehearse it until it takes 90 seconds.

#### Setup

Two terminals, two irssi instances with separate config directories so they do
not fight over the same session:

```
# terminal 1 — the sender
$ mkdir -p /tmp/irssi-alice
$ irssi --home=/tmp/irssi-alice

# terminal 2 — the receiver
$ mkdir -p /tmp/irssi-bob
$ irssi --home=/tmp/irssi-bob
```

Make a file to send, big enough that the transfer is visible but small enough to
be instant:

```
$ head -c 200000 /dev/urandom > /tmp/testfile.bin
$ ls -l /tmp/testfile.bin
$ md5 /tmp/testfile.bin        # macOS.  On Linux: md5sum
```

**Record that checksum.** Proving the received file matches byte-for-byte is the
strongest possible demo.

#### Connect both clients

In each irssi:
```
/connect 127.0.0.1 6667 <password>
/nick alice          (and bob in the other)
```

irssi sends `PASS`, `NICK`, `USER` for you. Confirm both are registered — you
should see your `001`–`004` welcome numerics.

#### Configure DCC for a local test

This is the step that trips people up. By default irssi may advertise a LAN or
public IP that the other local instance cannot reach, and may refuse to
auto-accept. In **both** instances:

```
/set dcc_own_ip 127.0.0.1
/set dcc_autoget ON
/set dcc_autoget_max_size 0
/set dcc_download_path /tmp/dcc-received
```

Then create the download directory:
```
$ mkdir -p /tmp/dcc-received
```

`dcc_own_ip 127.0.0.1` is the critical one: it forces irssi to advertise
`2130706433` in the offer, which the other local irssi can actually connect to.

#### Do the transfer

In alice's irssi:
```
/dcc send bob /tmp/testfile.bin
```

In bob's irssi you should see the incoming offer, and with `dcc_autoget ON` the
transfer starts immediately. Both sides show progress; on completion irssi
prints a summary line.

#### Prove it worked

```
$ ls -l /tmp/dcc-received/testfile.bin
$ md5 /tmp/dcc-received/testfile.bin
```

Same size, same checksum as the original. **That is your demo.** The file went
from alice to bob, and your server relayed the handshake that made it possible.

#### Watch the handshake go through your server

This is the part that actually demonstrates *your code*, and it is what you
should show the evaluator. In alice's irssi, before sending, turn on raw
logging:

```
/set show_raw_messages ON
```
or open the raw window with `/window new hidden` and `/raw`. Depending on your
irssi version, the reliable way is:
```
/log open -targets * /tmp/irssi-raw.log
```

Simpler and more convincing: **add temporary logging to your own server**
(§2.7), so the offer prints on your server's stdout as it passes through. An
evaluator watching your server terminal print

```
[DCC] alice -> bob : SEND testfile.bin from 127.0.0.1:52341 (200000 bytes)
```

at the exact moment the transfer starts has seen your server do its job.

#### Manual `nc` fallback

If irssi is unavailable or misbehaving, you can drive the handshake by hand and
still prove the relay. Two `nc` sessions:

```
# bob's terminal
$ nc -C 127.0.0.1 6667
PASS pass
NICK bob
USER bob 0 * :Bob

# alice's terminal
$ nc -C 127.0.0.1 6667
PASS pass
NICK alice
USER alice 0 * :Alice
```

Now, from alice, send a real DCC offer. The `\x01` bytes must be actual byte
0x01, so use `printf` rather than typing:

```
$ printf 'PRIVMSG bob :\001DCC SEND testfile.bin 2130706433 52341 200000\001\r\n' | nc -C 127.0.0.1 6667
```

(Register on that connection first, or script the whole sequence into one
`printf`.)

In bob's terminal you should see, byte for byte:

```
:alice!alice@127.0.0.1 PRIVMSG bob :^ADCC SEND testfile.bin 2130706433 52341 200000^A
```

Pipe bob's `nc` through `cat -v` or `hexdump -C` to make the `\x01` visible:

```
$ nc -C 127.0.0.1 6667 | cat -v
```
`cat -v` renders byte 0x01 as `^A`. Seeing `^A` on both ends proves the CTCP
markers survived your server intact — a very direct, very convincing
demonstration that takes ten seconds.

For total rigour, `hexdump -C` shows the literal `01` bytes:
```
$ nc -C 127.0.0.1 6667 | hexdump -C | grep -A2 'DCC'
```

### 2.6b Complete worked example — transfer a real file with only `nc`

The irssi route in §2.6 is the realistic demo. But you can perform a **complete,
genuine DCC SEND by hand**, moving actual file bytes, using nothing but `nc` and
your server. This is the best way to *understand* DCC, and it is a superb
fallback if irssi misbehaves on the evaluation machine.

You are going to play both clients manually. Remember the architecture: your
server relays the offer, the two "clients" connect directly. Here, `nc` is both
clients.

You need **five terminals**. Label them; it is easy to lose track.

---

#### Terminal 1 — the server

```
$ cd ~/Desktop/IRCSERV
$ make
$ ./ircserv 6667 pass
Arguments parsed successfully!
Port: 6667
Password: pass
Server <3> Listening on port 6667
Waiting to accept connections...
```

---

#### Terminal 2 — create the file and compute its facts

```
$ head -c 200000 /dev/urandom > /tmp/testfile.bin

$ ls -l /tmp/testfile.bin
-rw-r--r--  1 you  staff  200000 ... /tmp/testfile.bin

$ md5 /tmp/testfile.bin
MD5 (/tmp/testfile.bin) = 3f8a1c...      # <-- WRITE THIS DOWN
```

On Linux use `md5sum` instead of `md5`.

You need three numbers for the offer:

| Field | Value | How you got it |
|---|---|---|
| filesize | `200000` | `ls -l` |
| ip | `2130706433` | `127.0.0.1` encoded — see §2.3 |
| port | `52341` | you are about to pick it; any free high port |

Confirm the IP encoding yourself:
```
$ printf '%d\n' $((127*16777216 + 0*65536 + 0*256 + 1))
2130706433
```

---

#### Terminal 3 — bob connects to the IRC server and waits

```
$ nc -C 127.0.0.1 6667 | cat -v
```

Then type, line by line:
```
PASS pass
NICK bob
USER bob 0 * :Bob Receiver
```

You should immediately see your welcome numerics:
```
:ft_ircserv 001 bob :Welcome to the Internet Relay Network bob
:ft_ircserv 002 bob :Your host is ft_ircserv, running version 1.0
:ft_ircserv 003 bob :This server was created 2026
:ft_ircserv 004 bob ft_ircserv 1.0 o itkol
```

Leave this terminal open and watching. The `| cat -v` is what will render the
`\x01` CTCP bytes visibly as `^A`.

---

#### Terminal 4 — alice opens the file-serving socket

**This is the step that makes it a real transfer.** Alice's "client" must be
listening on the port she is about to advertise, ready to hand over the file to
whoever connects:

```
$ nc -l 52341 < /tmp/testfile.bin
```

Notes:
- On macOS, `nc -l 52341` listens on port 52341. On some Linux `netcat` variants
  you need `nc -l -p 52341`. If `-l` complains, try `nc -l -p 52341`.
- `< /tmp/testfile.bin` feeds the file into the socket, so the first thing that
  connects receives the file's bytes.
- This command **blocks**, waiting for a connection. That is correct — leave it.

This socket is exactly what a real DCC-sending client creates in step 1 of §2.2.
You are doing by hand what irssi does automatically.

---

#### Terminal 5 — alice connects to IRC and sends the offer

```
$ nc -C 127.0.0.1 6667 | cat -v
```

Register:
```
PASS pass
NICK alice
USER alice 0 * :Alice Sender
```

Now send the DCC offer. **You cannot type `\x01`** — it is a control byte, not a
character on your keyboard. Two ways around it:

**Option A — Ctrl-V then Ctrl-A.** In most terminals, `Ctrl-V` (literal-next)
followed by `Ctrl-A` inserts a raw 0x01 byte. Type:
```
PRIVMSG bob :<Ctrl-V><Ctrl-A>DCC SEND testfile.bin 2130706433 52341 200000<Ctrl-V><Ctrl-A>
```
then Enter. Fiddly but it works.

**Option B — script the whole session with `printf`.** More reliable, and what
you should use at the defense. Kill terminal 5's `nc` and run this instead —
note the `sleep` that lets registration complete before the offer goes out:

```
$ { printf 'PASS pass\r\n';
    printf 'NICK alice\r\n';
    printf 'USER alice 0 * :Alice Sender\r\n';
    sleep 1;
    printf 'PRIVMSG bob :\001DCC SEND testfile.bin 2130706433 52341 200000\001\r\n';
    sleep 5;
  } | nc -C 127.0.0.1 6667 | cat -v
```

`printf` interprets `\001` as the literal byte 0x01. That is the whole trick.

---

#### What you should see

**In terminal 3 (bob), immediately:**

```
:alice!alice@127.0.0.1 PRIVMSG bob :^ADCC SEND testfile.bin 2130706433 52341 200000^A
```

Read that line carefully — it is the proof that your server did its job:

| What to check | Why it matters |
|---|---|
| `^A` appears at **both** ends | The `\x01` CTCP markers survived. Your server did not strip them |
| The prefix is `alice!alice@127.0.0.1` | Full `nick!user@host`, built by `Client::prefix()` |
| All five fields present and unmangled | `SEND`, filename, ip, port, size — no whitespace collapsing |
| Nothing truncated | The full line arrived — this is what [1.9](#19-sending-without-pollout-blocker--explicit-grade-0-clause) protects |

**In terminal 1 (your server):** nothing new — which is *correct*. Your server
relayed a `PRIVMSG` and has no idea it was a file offer. (Unless you added the
§2.7 logging, in which case you now see the `[DCC]` line — a much nicer demo.)

---

#### Now actually fetch the file

Bob has the offer. He reads the IP and port out of it — `2130706433` decodes to
`127.0.0.1`, port `52341` — and connects directly. **Open a sixth terminal**
(or reuse terminal 2):

```
$ nc 127.0.0.1 52341 > /tmp/received.bin
```

This connects straight to alice's listening `nc` from terminal 4. The file
streams across. When it finishes, terminal 4's `nc -l` exits on its own (it hit
EOF on the file), and so does this one.

If it does not exit by itself after a moment, Ctrl-C it — some `nc` builds hold
the connection open. The bytes have arrived either way.

---

#### Prove it worked

```
$ ls -l /tmp/testfile.bin /tmp/received.bin
-rw-r--r--  1 you  staff  200000 ... /tmp/testfile.bin
-rw-r--r--  1 you  staff  200000 ... /tmp/received.bin

$ md5 /tmp/testfile.bin /tmp/received.bin
MD5 (/tmp/testfile.bin)  = 3f8a1c...
MD5 (/tmp/received.bin)  = 3f8a1c...      # identical
```

Or in one shot:
```
$ cmp /tmp/testfile.bin /tmp/received.bin && echo "IDENTICAL — transfer complete"
IDENTICAL — transfer complete
```

**That is a complete DCC file transfer**, negotiated through your IRC server,
with the file bytes flowing directly between the two peers. Exactly how real IRC
does it.

---

#### The one-screen version for the defense

Once you have done it slowly, compress it. Put this in a script — the subject
explicitly permits test programs (*"You are encouraged to create test programs
for your project even though they will not be submitted or graded... free to use
whatever tests you need during the evaluation process"*, Chapter VII):

```
#!/bin/bash
# dcc_demo.sh — end-to-end DCC transfer through ./ircserv
# usage: ./dcc_demo.sh <port> <password>
PORT=$1
PASS=$2

head -c 200000 /dev/urandom > /tmp/testfile.bin
echo "original: $(md5 -q /tmp/testfile.bin)"

# alice serves the file on 52341
nc -l 52341 < /tmp/testfile.bin &

# bob connects to IRC and logs what he receives
{ printf 'PASS %s\r\nNICK bob\r\nUSER bob 0 * :Bob\r\n' "$PASS"; sleep 6; } \
    | nc -C 127.0.0.1 "$PORT" | cat -v > /tmp/bob.log &

sleep 1

# alice connects to IRC and sends the DCC offer
{ printf 'PASS %s\r\nNICK alice\r\nUSER alice 0 * :Alice\r\n' "$PASS"
  sleep 1
  printf 'PRIVMSG bob :\001DCC SEND testfile.bin 2130706433 52341 200000\001\r\n'
  sleep 3
} | nc -C 127.0.0.1 "$PORT" > /dev/null &

sleep 2
echo "--- offer as bob received it ---"
grep DCC /tmp/bob.log

# bob acts on the offer
nc 127.0.0.1 52341 > /tmp/received.bin
echo "received: $(md5 -q /tmp/received.bin)"
cmp /tmp/testfile.bin /tmp/received.bin && echo "IDENTICAL — transfer complete"
wait
```

Rehearse it. Know what every line does — you will be asked. Note that this is a
**test script**, not part of your submission; keep it out of the graded sources
or in a clearly-marked `tests/` directory.

---

### 2.7 Optional: DCC-aware logging

Everything above works with your server unmodified. But "I didn't have to change
anything" is a weak-sounding answer even when it is the right one. A small,
**passive** addition makes the feature visible and gives you something concrete
to point at.

**The rule: log only. Never modify, never block, never reject.** The moment your
server makes a routing decision based on payload content, it stops being an IRC
server.

#### What to add

In `HandlePrivmsg`, after `formattedMsg` is built and **after** the message has
been relayed, add a detection-and-log step:

1. Test whether `text` begins with `\x01DCC ` and ends with `\x01`. If not,
   return — this is ordinary chat.
2. Strip the two `\x01` bytes.
3. Split the remainder on spaces into: `DCC`, subcommand, filename, ip, port,
   size.
4. Convert the IP integer back to dotted-quad. Write a small helper — divide by
   `16777216`, `65536`, `256` as shown in §2.3, or use `ntohl` plus
   `inet_ntoa` (both whitelisted).
5. Print a line to `std::cout` using your existing colour macros from
   `Server.hpp` (`GRE`, `YEL`, `WHI`), matching the style of your existing
   connect/disconnect logs.

Target output:

```
[DCC] alice -> bob : SEND testfile.bin from 127.0.0.1:52341 (200000 bytes)
```

#### Implementation notes

- **Filenames with spaces** are quoted: `"my file.pdf"`. Handle the quoted form
  or explicitly note that you log unquoted names only.
- **Do this after relaying, not before.** If your parser has a bug, the message
  must still get through. Logging is observability, not a gate.
- **Do not use `sscanf`** — not on the whitelist. Use `std::istringstream`,
  which is C++98 and allowed under "Everything in C++ 98".
- **`std::atoi` / `std::strtoul`** are C++98 (`<cstdlib>`) and fine for the
  numeric fields.
- Keep it in a new file, `srcs/commands/Dcc.cpp` or a helper in `Privmsg.cpp`,
  and add it to `SRCS`. Roughly 40 lines.

#### Why this earns points

- It proves you **understand** the payload rather than blindly forwarding bytes.
- It gives the evaluator something to *watch* during the transfer.
- It stays architecturally correct: the server still does not touch the file.
- It is a natural place to be asked "so what would you have to change to block
  `.exe` transfers?" — to which the answer is "parse the filename in this exact
  spot and drop the PRIVMSG, but that is a policy an IRC server should not have."

### 2.8 The stronger option: bot-mediated transfer

If you want file transfer that is unambiguously *yours* rather than a
correctly-behaving relay, route it through the bot. Since the bot is a client, it
can legitimately terminate a DCC connection.

**The flow:**

1. Alice: `/dcc send ircbot report.pdf` — a normal DCC offer to the bot.
2. The bot receives the CTCP offer through your server, parses it, and
   `connect`s directly to alice's advertised IP and port.
3. The bot `recv`s the bytes and writes them with `std::ofstream` into a
   storage directory.
4. The bot `PRIVMSG`s the channel: `Stored report.pdf (180 KB). Get it with !get report.pdf`.
5. Bob: `!get report.pdf` in the channel.
6. The bot opens its own listening socket, sends bob a `DCC SEND` offer for the
   stored file, accepts his connection, and streams it out.

**What this demonstrates:** both directions of DCC, real file I/O, and a
non-trivial bot. It is the most complete answer to "handle file transfer" you
can give.

**What it costs:**

- The bot needs `bind`/`listen`/`accept` **and** `connect` — a second and third
  socket alongside its server connection. All whitelisted.
- The bot's `poll()` set grows: the server connection, the listening socket, and
  each active transfer. Same non-blocking discipline as the server. Budget
  4–6 hours on top of the basic bot.
- Ephemeral-port and firewall issues on a lab machine can eat an afternoon.
- More surface area to break — and remember, a crashing bot at the defense is a
  bad look even if the mandatory part is untouched.

**Recommendation.** Do §2.5 + §2.6 + §2.7 first and get them solid — that is a
complete, defensible, correct answer to "handle file transfer", and it is what
the subject actually asks for. Build §2.8 only if the basic bot is finished,
tested, and you have a full day spare. A working simple bonus beats a broken
ambitious one, and the grading is binary either way.

Section [4.9](#49-optional-hooks-for-bot-mediated-transfer) notes where in the
bot's structure this would attach if you decide to go for it.

---

## Part 3 — The bot: design and architecture

> A note on how this and the next part are written. Per Chapter III of the
> subject — *"Only use AI-generated content that you fully understand and can
> take responsibility for"* — this guide gives you **structure, mechanics, and
> the reasoning behind every decision**, not a file to paste. Signatures, state
> machines, and the tricky mechanical bits (byte-order conversion, framing,
> `POLLOUT` discipline) are spelled out exactly. The logic in between is
> described precisely enough to write in one sitting, and writing it yourself is
> the point — you will be asked to justify it line by line.

### 3.1 What the bot must do

Minimum viable bot, matching what you picked:

| Requirement | Detail |
|---|---|
| Connect to your server | Over real TCP, as a normal client |
| Register | `PASS` → `NICK` → `USER`, then wait for `001` |
| Join a channel | `JOIN #<channel>` after registration completes |
| Stay alive | Reply `PONG` to every `PING` |
| Listen | Watch channel messages **and** direct messages for `!` commands |
| Respond | `!help`, `!time`, `!ping` |
| Exit cleanly | Send `QUIT`, close the socket, free everything, on SIGINT |

Invocation:
```
./ircbot <host> <port> <password> [nick] [channel]
```
Defaults for the optional two: `ircbot` and `#bot`. Taking host/port/password as
arguments matters — hardcoding `6667` means you cannot demo on whatever port the
evaluator picks, and they *will* pick a different one.

### 3.2 The bot is a client — the protocol from the other side

You have spent this project writing the server half. The bot is the mirror
image, and the symmetry is the best way to hold it in your head:

| Concern | `ircserv` (server) | `ircbot` (client) |
|---|---|---|
| Socket setup | `socket` → `setsockopt` → `bind` → `listen` → `accept` | `socket` → `connect` |
| Direction | many peers, one per fd | exactly one peer |
| Framing | accumulate into `Client::_buffer`, split on `\n` | **identical** — accumulate, split on `\n` |
| Parsing | `ParseLine` → `struct command` | **identical** — reuse `ParseLine` |
| Registration | *validates* `PASS`/`NICK`/`USER` | *sends* `PASS`/`NICK`/`USER` |
| `PING`/`PONG` | replies `PONG` to client `PING` | replies `PONG` to server `PING` |
| Numerics | *emits* `001`, `433`, `464`… | *interprets* `001`, `433`, `464`… |
| Multiplexing | `poll()` over many fds | `poll()` over one fd (two or more if you do §2.8) |

**Two of those rows say "identical", and that is a design decision waiting to be
made:** the framing loop and the parser are the same problem on both sides.
See [3.5](#35-reusing-the-servers-parser).

The other rows invert. Everywhere your server *validates* something, the bot
*produces* it. This is why building the bot is such a good check on the server —
if your server's `001` is malformed, your bot will hang waiting for a welcome it
never recognises, and you will have found a real mandatory-part bug.

### 3.3 Files and structure

```
IRCSERV/
├── Makefile              (gains a bonus target)
├── includes/
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   ├── Command.hpp       <-- shared with the bot
│   ├── Replies.hpp
│   └── Bot.hpp           <-- new
├── srcs/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── Command.cpp       <-- shared with the bot
│   └── commands/…
└── bot/                  <-- new
    ├── main.cpp          argument parsing, signals, construct + run
    ├── Bot.cpp           connection, poll loop, framing, dispatch
    └── BotCommands.cpp   the !command handlers
```

Splitting `Bot.cpp` from `BotCommands.cpp` mirrors how you split
`Server.cpp` from `srcs/commands/` — the transport and the command logic are
different concerns, and the consistency reads well.

`includes/Bot.hpp` sketch:

```
class Bot {
private:
    int         _fd;
    std::string _host;
    int         _port;
    std::string _password;
    std::string _nick;
    std::string _channel;

    std::string _inBuffer;      // accumulates recv()'d bytes, drained on '\n'
    std::string _outBuffer;     // pending writes, drained on POLLOUT
    bool        _registered;    // set when 001 arrives
    bool        _joined;        // set when our own JOIN echoes back
    static bool _signal;        // set by SIGINT handler

    // non-copyable: private, undefined  (C++98 idiom)
    Bot(const Bot&);
    Bot& operator=(const Bot&);

public:
    Bot(const std::string& host, int port, const std::string& password,
        const std::string& nick, const std::string& channel);
    ~Bot();

    void connectToServer();     // socket + connect + O_NONBLOCK
    void run();                 // the poll loop
    static void signalHandler(int);

private:
    void queueLine(const std::string& line);   // append to _outBuffer (+ CRLF)
    void flushOutput();                        // POLLOUT: partial-aware send
    void readFromServer();                     // POLLIN: recv + append
    void processBuffer();                      // drain complete lines
    void handleLine(const std::string& line);  // ParseLine + dispatch
    void sendRegistration();
    void reply(const std::string& target, const std::string& text);

    // BotCommands.cpp
    void handleBotCommand(const std::string& sender,
                          const std::string& target,
                          const std::string& text);
    void cmdHelp(const std::string& sender, const std::string& target);
    void cmdTime(const std::string& sender, const std::string& target);
    void cmdPing(const std::string& sender, const std::string& target);
};
```

Note the private-undefined copy constructor and assignment operator. That is the
C++98 way to make a class non-copyable (C++11's `= delete` is not available to
you), and it is exactly the fix suggested for `Server` in
[1.10j](#110-smaller-conformance-gaps). Using it here shows the idiom is
deliberate.

### 3.4 The connection lifecycle

The bot is a small state machine. Draw it before you write it:

```
   ┌──────────────┐
   │ DISCONNECTED │
   └──────┬───────┘
          │  socket() + connect()
          ▼
   ┌──────────────┐
   │  CONNECTED   │  queue PASS, NICK, USER
   └──────┬───────┘
          │  server replies 001 (RPL_WELCOME)
          ▼
   ┌──────────────┐
   │  REGISTERED  │  queue JOIN #channel
   └──────┬───────┘
          │  our own JOIN echoes back
          ▼
   ┌──────────────┐
   │   IN CHANNEL │◀──┐   normal operation:
   └──────┬───────┘   │   PING → PONG
          │           │   PRIVMSG → maybe a !command
          └───────────┘
          │  SIGINT, or ERROR from server
          ▼
   ┌──────────────┐
   │   QUITTING   │  queue QUIT, flush, close
   └──────────────┘
```

**The transitions that matter, and the mistakes to avoid:**

- **CONNECTED → REGISTERED is driven by `001`, not by a timer.** Do not
  `sleep` and hope. Do not send `JOIN` immediately after `USER`. Wait for the
  numeric. Your server sends `001` from `CheckRegistration`, so if the bot hangs
  here, `CheckRegistration` is the thing to debug.
- **You must handle `433` (nick in use).** If someone is already using `ircbot`,
  your bot must pick another nick — append a `_` or a digit and re-send `NICK` —
  or it will sit unregistered forever. An evaluator who happens to be logged in
  as `ircbot` would otherwise break your demo.
- **You must handle `464` (password mismatch).** Print a clear error and exit.
  Silently hanging on a wrong password is the single most confusing failure mode
  during a live demo.
- **`ERROR` from the server means the connection is finished.** Exit.
- **Do not act on messages before REGISTERED.** Ignore `!commands` until the
  handshake completes.

### 3.5 Reusing the server's parser

Both programs need to turn a line into `{prefix, command, params}`. You already
have that: `ParseLine` in `srcs/Command.cpp`, declared in `includes/Command.hpp`,
depending on nothing but `<string>` and `<vector>`.

**Compile `srcs/Command.cpp` into the bot as well.** Both binaries then parse IRC
identically, which is both less code and a stronger claim: "the bot and the
server share one parser, so there is no chance of them disagreeing about the
protocol."

Two things to know:

1. **It is not a violation of anything.** The file is yours, it is in your
   submission, and it links into both binaries. Reusing a translation unit across
   two targets is ordinary.
2. **Watch the object files.** If `srcs/Command.o` is built once and linked into
   both, that is fine and correct. Do not let your bonus target rebuild it with
   different flags into the same `.o` path — that is a classic source of
   "unnecessary relinking", which the subject forbids. [Part 5](#part-5--makefile-adding-the-bot-without-breaking-the-rules)
   handles this.

The framing loop is the other duplicated piece. It is ~10 lines and lives inside
`Server::ReceiveNewData`, entangled with `_clients` lookups, so extracting it is
more trouble than re-writing it. Write it again in `Bot::processBuffer` and be
ready to say why: *"the parser was cleanly separable and I share it; the framing
loop is coupled to the server's client map, so I reimplemented the same ten
lines rather than contorting the server's design for the bot's benefit."*

### 3.6 The fallback: a server-side bot

Your hedge if an evaluator reads *"You must not develop an IRC client"*
literally (see [0.3](#03-the-trap-you-must-not-develop-an-irc-client)). Keep this
in your back pocket; do not build it unless you must.

**The idea:** the bot becomes an object inside `Server` with no socket at all.

- Add a `Bot _bot;` member to `Server`, holding the bot's nick and its command
  table.
- Register a fake `Client` in `_clients` under a reserved negative fd (e.g. `-2`)
  so it can be a channel member and appear in `NAMES` — but never in `_fds`, so
  `poll()` never sees it.
- In `HandlePrivmsg`, before the normal target lookup, check whether the target
  is the bot's nick or whether the text starts with `!` in a channel the bot has
  joined. If so, call `_bot.handle(...)`.
- The bot "sends" by calling `SendReply` / `Channel::broadcast` directly — no
  socket, no `send` on its own behalf.

**Where it must be careful:**
- `SendReply(_bot.fd, ...)` must be a no-op. A `send()` on fd `-2` returns `-1`,
  harmless with `SIGPIPE` ignored, but guard it explicitly rather than relying on
  that.
- `ClearClients` and the disconnect paths must never remove the bot.
- `GetClientByNick` returning the bot means a real user can `PRIVMSG` it —
  which you want — but also `KICK` it. Decide and document the behaviour.

**Cost:** about two hours if the command handlers are already written and only
their transport changes. This is the real reason to keep `BotCommands.cpp`
separate from `Bot.cpp`: the command logic ports across unchanged, and only the
transport is rewritten.

**Say this at the defense if you pivot:** *"I built it as a separate binary
because `connect` is on the allowed-functions list and the bonus explicitly asks
for a bot. If you read the client prohibition strictly, here is the same bot
running inside the server with no socket of its own."* Having both ready is a
very strong position — but only attempt it if the separate binary is already
done.

---

## Part 4 — Building the bot, step by step

Build in this order. After every step the bot must compile and do something
observable. Do not write all eleven sections and then run it — you will spend
the evening bisecting.

### 4.1 Arguments and entry point

`bot/main.cpp`. Mirror the shape of `srcs/main.cpp`, which already does argument
validation well.

**Usage:**
```
./ircbot <host> <port> <password> [nick] [channel]
```

**Steps:**

1. Accept 4 to 6 arguments. Fewer or more → usage message to `std::cerr`,
   `return 1`.
2. Validate the port with the same logic as `isValidPort` in `srcs/main.cpp`:
   all digits, `1024 <= p <= 65535`. **Reuse the exact function** — copy it, or
   better, note in your README that both entry points validate ports
   identically.
3. Validate the password non-empty (the server rejects whitespace in it, so the
   bot need not re-check, but rejecting an empty one early gives a better error).
4. Default `nick` to `"ircbot"` and `channel` to `"#bot"` when omitted. If a
   channel is given without a leading `#`, prepend one — a small kindness that
   prevents a confusing demo failure.
5. Install signal handlers **before** constructing the `Bot`:
   ```
   signal(SIGINT,  Bot::signalHandler);
   signal(SIGQUIT, Bot::signalHandler);
   signal(SIGPIPE, SIG_IGN);
   ```
   `SIGPIPE` must be ignored for exactly the reason it is in your server: writing
   to a socket the peer has closed raises `SIGPIPE`, whose default action kills
   the process. Ignoring it makes `send` return `-1` instead, which you handle.
6. Construct the `Bot`, call `connectToServer()`, call `run()`, all inside
   `try`/`catch (const std::exception&)`. Print errors in red using the same
   macros — but note those macros live in `Server.hpp`, which the bot should not
   include. Either move `RED`/`GRE`/`YEL`/`WHI` into a small shared header, or
   redefine them in `Bot.hpp`. Moving them is cleaner and is a one-line change to
   `Server.hpp`.

**Checkpoint:** `./ircbot` with no arguments prints usage and exits 1.
`./ircbot 127.0.0.1 99999 pass` rejects the port.

### 4.2 Opening the connection

`Bot::connectToServer()`. This is the only part of the bot that has no analogue
in your server — everything else you have written before, from the other side.

**Steps:**

1. `_fd = socket(AF_INET, SOCK_STREAM, 0);` → throw on `-1`.
2. Fill a `struct sockaddr_in`:
   ```
   std::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_port   = htons(_port);
   addr.sin_addr.s_addr = inet_addr(_host.c_str());
   ```
   `inet_addr` returns `INADDR_NONE` (`0xFFFFFFFF`) for a string it cannot parse.
   Check for it. If you want to accept hostnames like `localhost`, use
   `getaddrinfo` (whitelisted) and remember `freeaddrinfo` — but `inet_addr` with
   a dotted quad is enough for the defense and has no cleanup to leak.
3. `connect(_fd, (struct sockaddr*)&addr, sizeof(addr))` → throw on `-1` with a
   message naming the host and port. This is where "server isn't running" shows
   up, so make the message good.
4. **Now** set non-blocking:
   ```
   fcntl(_fd, F_SETFL, O_NONBLOCK);
   ```

**Why in that order** — connect first, then `O_NONBLOCK`:

A non-blocking `connect()` does not complete immediately. It returns `-1` with
`errno == EINPROGRESS`, and you must then `poll` for `POLLOUT` and check
`getsockopt(SO_ERROR)` to discover whether it succeeded. That is real extra
complexity for zero benefit here — the bot has nothing else to do while
connecting.

By connecting **while still blocking** and switching to `O_NONBLOCK` immediately
after, you get a simple, correct connect and a fully non-blocking socket for the
entire message loop, which is the part that matters.

Be ready for the question *"isn't your connect blocking?"*:

> "Yes, deliberately. The subject's non-blocking requirement is about the server
> not stalling while serving many clients. The bot has exactly one peer and
> nothing to do until it is connected, so a blocking connect is the correct
> trade. Every byte after that is non-blocking and gated by `poll`, which is
> where it matters. `fcntl` is used only as `F_SETFL, O_NONBLOCK`, as the subject
> requires."

That is an honest, defensible answer. Do not pretend it is non-blocking.

**Checkpoint:** start `./ircserv 6667 pass`, run `./ircbot 127.0.0.1 6667 pass`,
and the server prints `Client <N> Connected from IP 127.0.0.1`. The bot then
does nothing — correct, you have not written `run()` yet.

### 4.3 The poll loop

`Bot::run()`. Structurally identical to `Server::ServerRun`, but with a
one-element `pollfd` array — and, crucially, **it gets `POLLOUT` right from the
start**, which is the lesson from [1.9](#19-sending-without-pollout-blocker--explicit-grade-0-clause).

```
while (!_signal)
{
    struct pollfd pfd;
    pfd.fd      = _fd;
    pfd.events  = POLLIN;
    if (!_outBuffer.empty())
        pfd.events |= POLLOUT;      // <-- only when there is something to write
    pfd.revents = 0;

    int ret = poll(&pfd, 1, -1);
    if (ret == -1) { if (_signal) break; throw std::runtime_error("poll() failed"); }

    if (pfd.revents & (POLLHUP | POLLERR)) break;   // server went away
    if (pfd.revents & POLLIN)  readFromServer();
    if (pfd.revents & POLLOUT) flushOutput();
}
```

**The five things to notice:**

1. **`events` is rebuilt every iteration** from `_outBuffer.empty()`. This is the
   discipline that prevents the 100%-CPU spin described in 1.9 step 5. Nothing to
   write → no `POLLOUT` → `poll` blocks properly.
2. **`POLLIN` is handled before `POLLOUT`.** Read first, so a command that
   arrives this cycle can have its reply queued and flushed in the same cycle.
3. **`POLLHUP | POLLERR` is checked and breaks.** Your server currently does not
   do this ([1.2](#12-the-poll-loop-fd-skip-blocker)); the bot should, and doing
   it here makes the fix in the server obvious.
4. **Timeout is `-1`.** Block indefinitely. Do not poll on a timer and burn CPU.
   If you later add a periodic task, use a real timeout in milliseconds.
5. **`_signal` is checked both in the condition and after `poll` returns `-1`,**
   because `SIGINT` interrupts `poll` with `EINTR` and you want a clean exit, not
   a thrown exception. Same pattern as `Server::ServerRun`.

**Checkpoint:** the bot connects and sits idle at **0% CPU** in `top`. If it
spins, `events` is wrong — you have `POLLOUT` set unconditionally.

### 4.4 Output: queueLine and flushOutput

Never call `send()` anywhere else in the bot. Two functions own all output.

**`queueLine(const std::string& line)`**
1. Append `line` to `_outBuffer`.
2. Append `"\r\n"` if `line` does not already end with it. Same guard your
   `SendReply` uses.
3. Return. **Do not send.** The next `poll` cycle will see a non-empty
   `_outBuffer`, request `POLLOUT`, and flush.

**`flushOutput()`** — called only when `poll` reported `POLLOUT`:
1. `ssize_t n = send(_fd, _outBuffer.c_str(), _outBuffer.size(), 0);`
2. `n > 0` → `_outBuffer.erase(0, n)`. **Erase exactly `n`, not everything.**
3. `n <= 0` → the connection is broken; set a flag to exit the loop.
4. **Return.** Do not loop until `_outBuffer` is empty — if the kernel buffer is
   full, the next `poll` will tell you when to continue. Looping here is
   re-introducing blocking behaviour by hand.

Step 2 is the whole lesson of 1.9 in two lines. `send` on a non-blocking socket
returns how many bytes it *actually* took, which may be fewer than you offered.
Erasing the full buffer discards unsent bytes; erasing `n` is correct.

**Checkpoint:** add a throwaway `queueLine("PING :test")` after connecting.
Your server replies with the `PONG` numeric — visible in the raw bytes if you
temporarily print everything received.

### 4.5 Input: reading and framing

**`readFromServer()`** — called only when `poll` reported `POLLIN`:
1. `char buf[1024]; std::memset(buf, 0, sizeof(buf));`
2. `ssize_t n = recv(_fd, buf, sizeof(buf) - 1, 0);`
3. `n == 0` → server closed the connection cleanly. Exit the loop.
4. `n < 0` → error. Exit the loop. (You polled first, so this should not occur.)
5. `n > 0` → `_inBuffer.append(buf, n)`, then call `processBuffer()`.

**Use `append(buf, n)`, not `+= buf`.** The two-argument form copies exactly `n`
bytes. `+= buf` treats the buffer as a C string and stops at the first NUL —
which would silently corrupt any message containing one, and CTCP payloads are
precisely where odd bytes show up. Your server gets this right with
`std::string(buff, bytes)`; match it.

**`processBuffer()`** — the same loop as `Server::ReceiveNewData`:
```
while (true)
{
    size_t pos = _inBuffer.find('\n');
    if (pos == std::string::npos) break;         // incomplete line, wait for more

    std::string line = _inBuffer.substr(0, pos);
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    _inBuffer.erase(0, pos + 1);

    if (!line.empty())
        handleLine(line);
}
```

This is the partial-data handling the subject names explicitly in IV.3. The bot
needs it for the same reason the server does: TCP is a byte stream, and a single
`recv` may return half a line, or three and a half lines.

**Checkpoint:** print every framed line to `std::cout` with a `<< ` prefix. Run
the bot and watch your server's `001`–`004` arrive as four separate, clean lines.
If they arrive glued together or split mid-line, the framing is wrong.

### 4.6 The registration handshake

Right after `connectToServer()` succeeds, queue three lines in this order:

```
PASS <password>
NICK <nick>
USER <nick> 0 * :IRC Bot
```

**Details that matter:**

- **`PASS` first.** Your `HandlePass` rejects it with `462` once registered, and
  `CheckRegistration` will not complete without `_passOk`. Sending `NICK` first
  happens to work with your server, but `PASS` first is the convention and costs
  nothing.
- **`USER` takes four parameters.** Your `HandleUser` requires
  `cmd.params.size() >= 4` and reads `params[0]` as username and `params[3]` as
  realname. The middle two (`0` and `*`) are the historical hostname and
  servername fields, ignored by every modern server including yours. The realname
  must be a **trailing parameter** (`:IRC Bot`) so it may contain spaces.
- **Queue all three at once.** They will be flushed together on the next
  `POLLOUT`, arrive in one TCP segment, and your server's framing loop will split
  and process them in order. This is a real test of your server's buffering — if
  the bot registers successfully from a single write, your framing is correct.
- **Do not send `JOIN` yet.** Wait for `001`. See §4.7.

**Checkpoint:** the server prints nothing unusual, and the bot receives
`001 ircbot :Welcome…`. If you get `464`, the password is wrong. If nothing
arrives, compare what you sent against what `HandleUser` expects — this is where
a malformed `USER` shows up.

### 4.7 Dispatching server messages

`handleLine(const std::string& line)`. Call `ParseLine(line)` — the shared parser
from `srcs/Command.cpp` — then branch on `cmd.command`.

Uppercase the command first, exactly as `Server::ParseCommands` does:
```
for (size_t i = 0; i < cmd.command.size(); ++i)
    cmd.command[i] = std::toupper(cmd.command[i]);
```

**The branches you need:**

| Incoming | What to do | Why |
|---|---|---|
| `PING` | `queueLine("PONG :" + cmd.params[0])` | **Non-negotiable.** Real servers disconnect clients that miss pings. Your server does not ping yet ([1.10e](#110-smaller-conformance-gaps)), but the bot must handle it to work against any server — and an evaluator may add pings as the "small modification" of Chapter VII |
| `001` | set `_registered = true`, then `queueLine("JOIN " + _channel)` | The only correct trigger for joining |
| `433` | nickname in use — append `_` to `_nick` and re-send `NICK` | Otherwise the bot hangs forever if someone holds the nick |
| `464` | password mismatch — print a clear error, exit | Otherwise you get a silent hang, the worst demo failure |
| `JOIN` | if the prefix's nick is your own, set `_joined = true` and log it | Confirms the join actually landed |
| `PRIVMSG` | extract sender, target, text → `handleBotCommand(...)` | The bot's actual job |
| `ERROR` | log it and exit the loop | The server is closing the link |
| `KICK` | if you are the target, optionally rejoin | Nice touch; shows you thought about it |
| anything else | ignore silently | Numerics you do not care about |

**Extracting the sender's nick from a prefix.** `cmd.prefix` is
`alice!alice@127.0.0.1`. The nick is everything before the first `!`:
```
std::string nick = cmd.prefix;
size_t bang = nick.find('!');
if (bang != std::string::npos)
    nick = nick.substr(0, bang);
```
Handle the case where there is no `!` — server-originated messages have a bare
server name as prefix.

**Working out where to reply.** Given `PRIVMSG <target> :<text>`:
- If `target` starts with `#` or `&`, it is a channel → reply **to the channel**.
- Otherwise `target` is the bot's own nick, i.e. a direct message → reply **to
  the sender's nick**.

That single rule is what makes the bot feel correct in both contexts, and it is
three lines:
```
std::string replyTo = (target[0] == '#' || target[0] == '&') ? target : senderNick;
```

**Checkpoint:** the bot joins `#bot` and appears in the member list when you
`/join #bot` from irssi. Send `hello` in the channel and watch the bot log it.

### 4.8 The bot commands

`bot/BotCommands.cpp`. Keeping these in their own translation unit is what makes
the [3.6](#36-the-fallback-a-server-side-bot) pivot cheap.

**`handleBotCommand(sender, target, text)`:**
1. Return immediately if `text` is empty or `text[0] != '!'`.
2. Take the first whitespace-delimited token as the command word; lowercase it.
3. Compute `replyTo` with the channel-vs-DM rule from §4.7.
4. Dispatch on the command word. Unknown `!command` → reply
   `"Unknown command. Try !help"`. Do **not** stay silent; silence looks broken.

**`!help`** — reply with one line listing the commands:
```
<ircbot> Commands: !help, !time, !ping
```
Keep it to a single `PRIVMSG`. Multi-line help means multiple messages, which is
where flooding starts.

**`!time`** — the current server time.
- `std::time(NULL)` from `<ctime>` gives a `time_t`.
- `std::localtime(&t)` gives a `struct tm*`.
- `std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm)` formats it.
- All three are C++98 (`<ctime>`), so they are covered by "Everything in C++ 98".
- **Do not use `std::put_time`** — that is C++11 and will not compile under
  `-std=c++98`.

**`!ping`** — reply `Pong!`. Trivial, but it is your liveness check: if `!ping`
answers, the whole path — server relay, bot framing, parsing, dispatch, output
queue — is working. It is the first thing to try when something breaks.

**Two safeguards worth adding:**

1. **Never respond to yourself.** If the sender's nick equals `_nick`, return.
   Without this, a bot whose reply happens to start with `!` will answer itself,
   forever, at line speed. This is the classic bot infinite loop, and it will
   take your server down during the demo.
2. **Rate-limit.** Track the timestamp of the last reply and drop commands
   arriving within, say, 500 ms. One user pasting `!ping` twenty times should not
   make the bot flood the channel. Mention this unprompted at the defense — it
   shows you thought past the happy path.

**Checkpoint:** from irssi in `#bot`, type `!help`, `!time`, `!ping`. Then
`/msg ircbot !time` and confirm the reply arrives as a direct message, not in the
channel.

### 4.9 Optional hooks for bot-mediated transfer

If you decide to build §2.8, here is where it attaches. **Only start this once
§4.1–4.8 are finished and tested.**

- **Detect the offer.** In `handleBotCommand`, before the `!` check, test whether
  `text` starts with `\x01DCC SEND ` and ends with `\x01`. Parse it with
  `std::istringstream` into filename, ip, port, size (§2.3).
- **Decode the IP.** `ntohl` plus `inet_ntoa`, or the arithmetic from §2.3. Both
  routes are whitelisted.
- **Fetch the file.** A second `socket` + `connect` to that ip and port, added to
  the bot's `pollfd` array. On `POLLIN`, `recv` and append to a
  `std::ofstream` opened in binary mode (`std::ios::binary` — mandatory, or you
  corrupt every non-text file). Stop at `filesize` bytes.
- **Grow the poll set.** `run()` moves from one `pollfd` to a
  `std::vector<pollfd>` — the bot's server connection, plus one entry per active
  transfer. The same rebuild-`events`-every-cycle discipline applies, and now the
  same fd-erasure hazard from [1.2](#12-the-poll-loop-fd-skip-blocker) applies to
  the bot too. Defer removals to the end of the cycle from the very first
  version; do not repeat the server's mistake.
- **Serve it back.** For `!get <file>`: `socket` + `bind(0)` (port 0 = let the
  kernel choose) + `listen`, then `getsockname` (whitelisted, and this is what it
  is for) to discover which port you got, then `PRIVMSG` the requester a
  `DCC SEND` offer naming that port, then `accept` and stream the file out with
  `std::ifstream` in binary mode.
- **Cap the size.** A `!send` of a 4 GB file will fill the disk. Reject anything
  over a few MB and say so in the reply.

This is a solid afternoon of work, most of it in the poll-set refactor. Budget
accordingly and keep the simple version working in a separate commit you can fall
back to.

### 4.10 Signals and clean shutdown

Mirror your server exactly. `static bool Bot::_signal`, set by
`Bot::signalHandler`, checked by `run()`'s loop condition.

**On shutdown, in order:**
1. `queueLine("QUIT :Bot shutting down")`.
2. Flush it — one direct, blocking `send` of the remaining `_outBuffer` is
   acceptable here and is the same trade-off discussed in 1.9's "ordering detail".
   The alternative is polling until the buffer drains, which is cleaner but needs
   a timeout so a dead server cannot hang your exit.
3. `close(_fd)`.
4. Let the destructor release everything else.

**Why bother sending `QUIT`?** Because it exercises your server's
`DisconnectClient` path and makes other channel members see the bot leave. It is
a two-line way to demonstrate that path working, live.

**A useful thing to keep in mind:** the only thing a signal handler may safely do
is set a `volatile sig_atomic_t` flag. Your server declares `_Signal` as a plain
`static bool`, which is what nearly every ft_irc does and what evaluators accept
— but if asked "is that signal-safe?", the honest answer is *"strictly,
`volatile sig_atomic_t` is the type the standard guarantees; `bool` works on
every platform I target, and the handler does nothing but set the flag, which is
the safe pattern."* Knowing the nuance beats not knowing it.

**Checkpoint:** Ctrl-C the bot. The server logs the disconnect with the QUIT
reason, and irssi in `#bot` shows the bot leaving. No leaks under valgrind/ASan.

### 4.11 Whitelist audit for the bot

Before you call the bot done, verify every system call it makes appears on the
page-6 list from [0.4](#04-the-allowed-functions-whitelist).

| Call | On the list? | Used for |
|---|---|---|
| `socket` | ✅ | creating the connection |
| `connect` | ✅ | reaching the server |
| `close` | ✅ | teardown |
| `send` / `recv` | ✅ | the message stream |
| `poll` | ✅ | multiplexing |
| `fcntl` | ✅ **restricted** | only `fcntl(fd, F_SETFL, O_NONBLOCK)` |
| `htons` | ✅ | port byte order |
| `inet_addr` | ✅ | host string → address |
| `ntohl` / `inet_ntoa` | ✅ | DCC ip decoding, if you do §2.8 |
| `getsockname` | ✅ | discovering your `bind(0)` port, if you do §2.8 |
| `signal` | ✅ | SIGINT / SIGPIPE |
| `std::ifstream` / `std::ofstream` | ✅ via "Everything in C++ 98" | file I/O, if you do §2.8 |
| `std::time` / `localtime` / `strftime` | ✅ via "Everything in C++ 98" | `!time` |
| `std::istringstream` | ✅ via "Everything in C++ 98" | parsing |

**Not allowed — check you used none of them:**

- `open`, `read`, `write` — **not on the list**. Use C++ streams and `send`/`recv`
- `fork`, `exec*` — explicitly prohibited
- `sscanf` — not on the list; use `std::istringstream`
- `strdup`, `getopt`, `select` in place of `poll` (equivalents are allowed, but be
  consistent)
- anything from Boost or any external library

Run this to catch strays:
```
$ grep -nE '\b(open|read|write|fork|exec|sscanf|strdup|getopt)\s*\(' bot/ srcs/
```
Review each hit. `std::ifstream::read` and `std::ostream::write` are C++ stream
methods, not the POSIX calls, and are fine — but you should be able to say which
is which when the grep result is on screen.

---

## Part 5 — Makefile: adding the bot without breaking the rules

### 5.1 The constraints

From Chapter II:

> - You have to turn in a `Makefile` which will compile your source files. It
>   must not perform **unnecessary relinking**.
> - Your `Makefile` must at least contain the rules: `$(NAME)`, `all`, `clean`,
>   `fclean` and `re`.

Note what is **not** there: **`bonus` is not a required rule** in subject v10.0.
Many 42 subjects mandate one; this one does not. Adding it is still the right
call — it is the conventional place an evaluator looks — but you are not
penalised for the naming, only for relinking and for missing the five required
rules.

Your current Makefile has all five and is clean. The risk is entirely in what
you add.

### 5.2 What to add

Four changes:

1. **A second `NAME`:**
   ```
   NAME     = ircserv
   BOT_NAME = ircbot
   ```
2. **A separate source list for the bot**, including the shared parser:
   ```
   BOT_SRCS = bot/main.cpp \
              bot/Bot.cpp \
              bot/BotCommands.cpp \
              srcs/Command.cpp
   BOT_OBJS = $(BOT_SRCS:.cpp=.o)
   ```
3. **A `bonus` rule** that builds only the bot:
   ```
   bonus: $(BOT_NAME)

   $(BOT_NAME): $(BOT_OBJS)
   	$(CXX) $(CXXFLAGS) $(BOT_OBJS) -o $(BOT_NAME)
   ```
4. **Extend `clean` / `fclean` / `.PHONY`:**
   ```
   clean:
   	rm -f $(OBJS) $(BOT_OBJS)

   fclean: clean
   	rm -f $(NAME) $(BOT_NAME)

   .PHONY: all bonus clean fclean re
   ```

Your existing pattern rule `%.o: %.cpp` already covers `bot/*.cpp` — it is
generic, so no new compilation rule is needed. That is why the bot's sources drop
in cleanly.

### 5.3 The relinking trap — and the one real gotcha here

`srcs/Command.cpp` appears in **both** `SRCS` and `BOT_SRCS`. Both lists derive
their objects with `$(...:.cpp=.o)`, so both name the same file:
`srcs/Command.o`.

**This is fine, and it is what you want.** Make builds `srcs/Command.o` once, and
links the same object into both binaries. Make's dependency graph handles the
sharing correctly: the object is newer than its source, so the second target does
not rebuild it.

**Where it goes wrong** is if you ever compile the shared file with different
flags per target — for instance adding `-DBOT_BUILD` to the bot's compilation.
Then each `make` alternates, rebuilding `srcs/Command.o` with whichever flags ran
last, and *both* binaries relink every time. That is precisely the "unnecessary
relinking" the subject forbids.

**Rule: keep `CXXFLAGS` identical for both targets.** If you ever genuinely need
different flags for a shared file, give the bot its own object path (e.g.
`bot/Command.o` built from `srcs/Command.cpp` via an explicit rule) so the two
never collide.

### 5.4 Verify it

The subject's requirement is testable in about thirty seconds. Do it before the
defense.

```
$ make fclean
$ make                    # builds ircserv
$ make                    # MUST say "make: `ircserv' is up to date."
$ make bonus              # builds ircbot; must NOT rebuild ircserv
$ make bonus              # MUST say "make: `ircbot' is up to date."
$ make                    # MUST still say ircserv is up to date
$ make bonus              # and ircbot still up to date
```

That last pair is the one that catches the shared-object problem. Alternating
`make` and `make bonus` repeatedly must produce no compilation at all after the
first build of each. If you see `srcs/Command.o` being rebuilt on the alternation,
you have the §5.3 problem.

Also confirm:
```
$ make fclean && ls
```
No `.o` files anywhere, no `ircserv`, no `ircbot`.

And confirm the bot compiles under the required flags with no warnings — it is
built by the same `CXXFLAGS`, so `-Wall -Wextra -Werror -std=c++98` applies. A
single warning in `bot/` fails the build, which is what you want.

### 5.5 Should `all` build the bot?

**No.** Keep `all: $(NAME)`.

Reasons:
- The subject's `all` is about the mandatory deliverable.
- If the bot ever fails to compile, `make` must still produce a working
  `ircserv`. Given [0.2](#02-the-rule-that-catches-everyone), never let bonus
  code stand between the evaluator and a building mandatory part.
- It makes the isolation argument from [0.3](#03-the-trap-you-must-not-develop-an-irc-client)
  concrete: you can `rm -rf bot/`, run `make`, and the mandatory part builds and
  runs untouched. Offer to demonstrate exactly that if the client-prohibition
  question comes up.

---

## Part 6 — Testing everything

The subject encourages this explicitly (Chapter VII):

> You are encouraged to create test programs for your project even though they
> **will not be submitted or graded**. Those tests could be especially useful to
> test your server during defense... you are free to use whatever tests you need
> during the evaluation process.

So write them, keep them in a `tests/` directory, and **do not** count them as
submission sources.

### 6.1 The full run-through

Do this end to end the night before the defense. Time it.

**Terminal 1 — server**
```
$ make fclean && make && make bonus
$ ./ircserv 6667 pass
```

**Terminal 2 — reference client (irssi)**
```
$ irssi --home=/tmp/irssi-a
/connect 127.0.0.1 6667 pass
/nick alice
/join #test
```

**Terminal 3 — second reference client**
```
$ irssi --home=/tmp/irssi-b
/connect 127.0.0.1 6667 pass
/nick bob
/join #test
```

**Terminal 4 — the bot**
```
$ ./ircbot 127.0.0.1 6667 pass ircbot '#test'
```

Now walk the checklist below with all four running.

### 6.2 Mandatory-part regression list

Re-run this after every bonus change. The bonus cannot save a broken mandatory
part.

| Test | Expected |
|---|---|
| Wrong password | irssi cannot register; `464` |
| `JOIN #test` from both | both appear in `/names`; each sees the other's join |
| `PRIVMSG` in channel | the other client receives it, sender does not echo |
| `/msg bob hi` | direct message arrives; nobody else sees it |
| `/topic #test :hello` as non-op with `+t` | `482` |
| `/mode #test -t` then non-op sets topic | succeeds, all members see `TOPIC` |
| `/mode #test +k secret` | a third client joining without the key gets `475` |
| `/mode #test +l 2` | third client gets `471` |
| `/mode #test +i` | uninvited client gets `473`; after `/invite`, join succeeds |
| `/mode #test +o bob` | bob can now `KICK` |
| `/kick #test bob` | bob leaves; all members see it |
| `/quit` | others see the `QUIT` |
| `kill -9` on an irssi | others **still** see the `QUIT` ([1.3](#13-silent-disconnects-never-reach-the-channel-blocker)) |
| Last member parts | channel is destroyed (check with a fresh `JOIN` — it should be a new channel with you as op) |
| `nc -C` with `com`^D`man`^D`d` | rebuilt as one command ([1.11](#111-subject-compliance-audit)) |
| Server idle with clients connected | 0% CPU |
| Ctrl-C the server | clean exit, all fds closed, no leaks |

### 6.3 Bonus test list

| Test | Expected |
|---|---|
| Bot connects and joins | appears in `/names #test` |
| `!help` in channel | replies **in the channel** |
| `!time` in channel | correct current time |
| `!ping` in channel | `Pong!` |
| `/msg ircbot !time` | replies **directly**, not in the channel |
| `!nonsense` | "Unknown command. Try !help", not silence |
| `!ping` pasted 20× fast | rate-limited; the channel does not flood |
| Server sends the bot a `PING` | bot replies `PONG`, stays connected |
| Second bot with the same nick | second one handles `433`, renames itself, still joins |
| Ctrl-C the bot | sends `QUIT`; channel members see it leave; no leaks |
| Bot started before the server | clean error message naming host and port, exit 1 — not a hang |
| Bot with a wrong password | clean `464` error, exit — not a hang |
| Kill the **server** while the bot runs | bot detects `POLLHUP`, exits cleanly, does not spin |
| DCC transfer between two irssi clients | file arrives; `cmp` says identical (§2.6) |
| Manual `nc` DCC transfer | offer relayed with `^A` intact; `cmp` identical (§2.6b) |

That last-but-two row — killing the server out from under the bot — is worth
rehearsing. A bot that spins at 100% CPU or crashes when its peer vanishes is a
bad thing for an evaluator to notice.

### 6.4 Memory and fd checks

**Linux:**
```
$ valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./ircserv 6667 pass
$ valgrind --leak-check=full ./ircbot 127.0.0.1 6667 pass
```

**macOS** (valgrind is usually unavailable on Apple Silicon):
```
$ c++ -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address,undefined -I./includes $(SRCS) -o ircserv_asan
$ ./ircserv_asan 6667 pass
```
or
```
$ leaks --atExit -- ./ircserv 6667 pass
```

**File descriptor leaks** — connect and disconnect fifty clients, then check the
count has returned to its baseline:
```
$ lsof -p $(pgrep -f 'ircserv 6667') | wc -l
```
Run it before and after. A steadily climbing number means `close()` is being
missed on some path — most likely one of the disconnect routes from
[1.3](#13-silent-disconnects-never-reach-the-channel-blocker).

### 6.5 A stress script worth having

```
#!/bin/bash
# tests/stress.sh — 50 clients connect, register, join, talk, and leave
PORT=${1:-6667}
PASS=${2:-pass}
for i in $(seq 1 50); do
  { printf 'PASS %s\r\nNICK user%d\r\nUSER user%d 0 * :User %d\r\nJOIN #stress\r\n' \
      "$PASS" "$i" "$i" "$i"
    sleep 2
    printf 'PRIVMSG #stress :hello from %d\r\n' "$i"
    sleep 1
    printf 'QUIT :bye\r\n'
  } | nc -C 127.0.0.1 "$PORT" > /dev/null &
done
wait
echo "done — check the server is still alive and at 0%% CPU"
```

Then confirm the server is still running, still responsive to a fresh irssi
connection, and back at 0% CPU with no leaked fds. This is the test that finds
[1.2](#12-the-poll-loop-fd-skip-blocker) if you have not fixed it — with fifty
clients disconnecting concurrently, the fd-skip bug becomes reliable rather than
rare.

---

## Part 7 — Defending it at evaluation

Chapter VII warns that you may be asked to modify the project live:

> During the evaluation, a brief **modification of the project** may occasionally
> be requested. This could involve a minor behavior change, a few lines of code
> to write or rewrite, or an easy-to-add feature... This step is meant to verify
> your actual understanding of a specific part of the project.

For `ft_irc`, the likely asks are: add a new command, add a new bot command,
change a numeric's text, or add a channel mode. If you built the bot yourself,
adding `!roll` or `!echo` live takes three minutes and is the best possible
demonstration. **Practise adding one bot command from scratch, timed.**

### 7.1 Questions you will be asked, with answers

**"Why is your bot a separate program? The subject says no IRC client."**
See [0.3](#03-the-trap-you-must-not-develop-an-irc-client) — scope, intent, and
the `connect` argument. Finish with: *"and `ircserv` contains zero bonus code —
I can delete `bot/`, run `make`, and the mandatory part is untouched. Want me to
show you?"*

**"How does file transfer work? Show me where your server sends the file."**
It doesn't, and that is correct. Draw the diagram from §2.1. *"IRC has no
file-transfer command. DCC is client-to-client; the server relays the CTCP
handshake in a PRIVMSG and the clients open a direct socket. My server's job is
to relay that payload without mangling it — here is the `^A` on both sides
proving the CTCP markers survived."*

**"Show me the DCC message."** Have §2.6b's `nc | cat -v` ready in a terminal.
Ten seconds, and it is completely convincing.

**"Why is the IP in the DCC message a huge number?"**
It is the IPv4 address as a 32-bit integer in host byte order. `2130706433` is
`127.0.0.1`. Show the arithmetic from §2.3.

**"What happens if `send()` returns fewer bytes than you gave it?"**
*"I keep the unsent remainder in the client's output buffer, request `POLLOUT`
for that fd on the next cycle, and erase exactly the number of bytes the kernel
accepted. When the buffer empties I stop requesting `POLLOUT`, otherwise `poll`
returns immediately forever and the server spins."* — Only say this once
[1.9](#19-sending-without-pollout-blocker--explicit-grade-0-clause) is actually
done.

**"How do you handle a command split across two TCP packets?"**
Show `Client::_buffer` and the framing loop. Then run the subject's own `nc -C`
+ Ctrl-D test from §1.11 live.

**"Where do your `Channel` objects store members, and what happens when a client
disconnects?"** `std::set<Client*>` pointing into `Server::_clients`, which is a
`std::map<int, Client>` — node-based, so the pointers stay valid. On disconnect,
`ClearClients` removes the client from every channel *before* erasing it from the
map, deletes any channel left empty, and drops the fd from the poll set.

**"What's the risk with `std::vector<Channel>`?"** Reallocation on `push_back`
invalidates every `Channel*`. `HandleJoin` re-fetches the pointer after creating
a channel for exactly this reason. Be honest that this is a sharp edge and say
what you would change (a `std::map<std::string, Channel>`, or storing
`Channel*`).

**"Why one `poll()` and not a thread per client?"**
The subject requires it — and the reason it requires it is that one `poll` over N
fds scales to thousands of connections with one stack and no synchronisation,
whereas a thread per client costs a stack each and needs locks around every
shared structure.

**"Is your `connect()` in the bot non-blocking?"**
No, deliberately. Give the §4.2 answer. Do not bluff.

**"Can a client do anything before authenticating?"**
No — and demonstrate it: `nc -C localhost 6667`, then `JOIN #x`, and show the
`451`. (Only true once [1.4](#14-unregistered-clients-can-run-any-command-blocker)
is fixed.)

### 7.2 The AI disclosure question

Chapter III is unusually direct, and its **bad practice** example is:

> I let Copilot generate my code for a key part of my project. It compiles, but I
> can't explain how it handles pipes. During the evaluation, I fail to justify
> and I fail my project.

Expect to be asked what you used AI for. Answer the way your README does
([1.0](#10-there-is-no-readmemd-blocker)) — specifically, by task and by part.
Being precise here reads as confidence; being vague reads as concealment.

The defense against this question is not rhetorical, it is preparation: be able
to explain any line in your project when a finger lands on it. That is the whole
reason this guide gives you structure and mechanics rather than a file to paste.

### 7.3 The morning-of checklist

- [ ] `git status` clean; everything committed and **pushed**
- [ ] `make fclean && make && make bonus` from a fresh clone in `/tmp`
- [ ] No `.o` files, no binaries, no `en.subject.pdf` in the repo
- [ ] `README.md` present and complete
- [ ] irssi installed and its config directories pre-made
- [ ] `/tmp/testfile.bin` created and its checksum written down
- [ ] The four-terminal layout from §6.1 rehearsed once
- [ ] `dcc_demo.sh` and `stress.sh` executable and tested
- [ ] valgrind/ASan run clean, output saved to show
- [ ] You can add a new bot command from scratch in under five minutes

---

## Appendix A — IRC message grammar

RFC 1459 §2.3.1, in the form your parser implements:

```
message  =  [ ":" prefix SPACE ] command [ params ] crlf
prefix   =  servername / ( nickname [ [ "!" user ] "@" host ] )
command  =  1*letter / 3digit
params   =  *14( SPACE middle ) [ SPACE ":" trailing ]
middle   =  any sequence not starting with ":" and containing no SPACE
trailing =  any sequence, may contain SPACE and ":"
crlf     =  CR LF
```

**The five rules that matter in practice:**

1. **Max 512 bytes per message, including the trailing `\r\n`.** This is the cap
   [1.8](#18-unbounded-input-buffer) asks you to enforce.
2. **Max 15 parameters.** Your `ParseLine` enforces this with
   `while (cmd.params.size() < 15)`.
3. **The trailing parameter starts at the first `:` in the parameter section and
   runs to end of line.** It may contain spaces and further colons. This single
   rule is why DCC works through your server unmodified (§2.5).
4. **The prefix is optional from client to server.** Clients normally omit it;
   servers always add it when relaying.
5. **Commands are case-insensitive.** Both your server and your bot uppercase
   before dispatch.

**Worked examples:**

| Wire | prefix | command | params |
|---|---|---|---|
| `NICK alice` | *(none)* | `NICK` | `["alice"]` |
| `USER alice 0 * :Alice R` | *(none)* | `USER` | `["alice","0","*","Alice R"]` |
| `JOIN #a,#b key1,key2` | *(none)* | `JOIN` | `["#a,#b","key1,key2"]` |
| `:bob!b@h PRIVMSG #a :hi :there` | `bob!b@h` | `PRIVMSG` | `["#a","hi :there"]` |
| `:ft_ircserv 001 alice :Welcome` | `ft_ircserv` | `001` | `["alice","Welcome"]` |
| `PRIVMSG bob :\x01DCC SEND f 2130706433 52341 100\x01` | *(none)* | `PRIVMSG` | `["bob","\x01DCC SEND f 2130706433 52341 100\x01"]` |

Note row 4: the trailing param is `hi :there`, one parameter containing a colon.
And row 6: the entire CTCP payload is one parameter. Both follow from rule 3.

---

## Appendix B — Numeric replies reference

Everything your server emits or should emit. The ones already built in
`includes/Replies.hpp` are marked ✅ — use those rather than hand-rolling
([1.5](#15-malformed-numeric-replies)).

**Registration**

| Num | Name | In Replies.hpp | Format |
|---|---|---|---|
| 001 | `RPL_WELCOME` | ✅ | `:<srv> 001 <nick> :Welcome to the Internet Relay Network <nick>` |
| 002 | `RPL_YOURHOST` | — | inline in `CheckRegistration` |
| 003 | `RPL_CREATED` | — | inline in `CheckRegistration` |
| 004 | `RPL_MYINFO` | — | inline in `CheckRegistration` |

**Channel replies**

| Num | Name | In Replies.hpp | Meaning |
|---|---|---|---|
| 324 | `RPL_CHANNELMODEIS` | ✅ | current modes, from `MODE #chan` |
| 331 | `RPL_NOTOPIC` | ✅ | no topic set |
| 332 | `RPL_TOPIC` | ✅ | the topic |
| 341 | `RPL_INVITING` | ✅ | invite acknowledged |
| 353 | `RPL_NAMREPLY` | ✅ | member list |
| 366 | `RPL_ENDOFNAMES` | ✅ | end of member list |

**Errors**

| Num | Name | In Replies.hpp | When |
|---|---|---|---|
| 401 | `ERR_NOSUCHNICK` | ✅ | target nick does not exist |
| 403 | `ERR_NOSUCHCHANNEL` | ✅ | channel does not exist |
| 404 | `ERR_CANNOTSENDTOCHAN` | ✅ | PRIVMSG to a channel you are not in |
| 411 | `ERR_NORECIPIENT` | ✅ | PRIVMSG with no target |
| 412 | `ERR_NOTEXTTOSEND` | ✅ | PRIVMSG with no text |
| 421 | `ERR_UNKNOWNCOMMAND` | ❌ **add it** | see [1.10g](#110-smaller-conformance-gaps) |
| 431 | `ERR_NONICKNAMEGIVEN` | — | inline in `Nick.cpp` |
| 432 | `ERR_ERRONEUSNICKNAME` | — | inline in `Nick.cpp` |
| 433 | `ERR_NICKNAMEINUSE` | — | inline in `Nick.cpp`; **the bot must handle this** |
| 441 | `ERR_USERNOTINCHANNEL` | ✅ | MODE/KICK target not in the channel |
| 442 | `ERR_NOTONCHANNEL` | ✅ | you are not in that channel |
| 443 | `ERR_USERONCHANNEL` | ✅ | INVITE target is already there |
| 451 | `ERR_NOTREGISTERED` | ✅ | command before registration ([1.4](#14-unregistered-clients-can-run-any-command-blocker)) |
| 461 | `ERR_NEEDMOREPARAMS` | ✅ | too few parameters |
| 462 | `ERR_ALREADYREGISTERED` | — | inline in `Pass.cpp` / `User.cpp` |
| 464 | `ERR_PASSWDMISMATCH` | ✅ | wrong password; **the bot must handle this** |
| 471 | `ERR_CHANNELISFULL` | ✅ | `+l` limit reached |
| 472 | `ERR_UNKNOWNMODE` | ✅ | unknown mode character |
| 473 | `ERR_INVITEONLYCHAN` | ✅ | `+i` and not invited |
| 475 | `ERR_BADCHANNELKEY` | ✅ | wrong or missing `+k` key |
| 482 | `ERR_CHANOPRIVSNEEDED` | ✅ | operator privileges required |

**The shape every numeric must have:**
```
:<servername> <3-digit> <target-nick> [<params>] :<human text>
```
The servername prefix and the target nick are **both mandatory**. That is the
whole content of [1.5](#15-malformed-numeric-replies).

---

## Appendix C — CTCP and DCC reference

### CTCP framing

A CTCP message is an ordinary `PRIVMSG` (or `NOTICE`) whose text is wrapped in
byte `0x01`:

```
PRIVMSG <target> :\x01<TAG> <arguments>\x01
```

Your server treats it as opaque text — which is correct, and is why DCC works
today.

**Common tags:**

| Tag | Example | Purpose |
|---|---|---|
| `ACTION` | `\x01ACTION waves\x01` | `/me` |
| `VERSION` | `\x01VERSION\x01` | ask a client what it is |
| `PING` | `\x01PING 1234567\x01` | client-to-client latency |
| `TIME` | `\x01TIME\x01` | ask a client its local time |
| `DCC` | `\x01DCC SEND ...\x01` | negotiate a direct connection |

Worth knowing: `/me` is CTCP too. If `/me` works through your server, so does
DCC — same mechanism, same code path. That is a nice one-line proof to offer.

### DCC subcommands

```
DCC SEND   <filename> <ip-as-uint32> <port> <filesize>
DCC CHAT   chat <ip-as-uint32> <port>
DCC RESUME <filename> <port> <position>
DCC ACCEPT <filename> <port> <position>
```

Filenames containing spaces are wrapped in double quotes:
```
DCC SEND "my report.pdf" 2130706433 52341 184320
```

### IP encoding

`a.b.c.d` → `a*16777216 + b*65536 + c*256 + d`

| Dotted | Integer |
|---|---|
| `127.0.0.1` | `2130706433` |
| `192.168.1.10` | `3232235786` |
| `10.0.0.1` | `167772161` |
| `0.0.0.0` | `0` |

Conversions:
```
$ python3 -c "import ipaddress;print(int(ipaddress.IPv4Address('192.168.1.10')))"
$ python3 -c "import ipaddress;print(ipaddress.IPv4Address(3232235786))"
$ printf '%d\n' $((192*16777216 + 168*65536 + 1*256 + 10))
```

In C++98 with whitelisted functions only:
- **integer → dotted:** `struct in_addr a; a.s_addr = htonl(value); inet_ntoa(a);`
- **dotted → integer:** `ntohl(inet_addr("127.0.0.1"))`

`htonl`/`ntohl` are on the allowed list specifically for conversions like these.

### The transfer protocol itself

Once connected, DCC SEND is almost trivially simple:

1. The sender writes the file's bytes to the socket, in order, no framing.
2. After each chunk, the receiver writes back the **total bytes received so far**
   as a 4-byte **big-endian** unsigned integer.
3. The sender may ignore these acknowledgements (most modern clients do).
4. The transfer ends when `filesize` bytes have arrived.

There is no checksum, no negotiation, no resume without the explicit
`RESUME`/`ACCEPT` exchange. It is a 1990s protocol and it shows. None of this is
your server's concern — but knowing it lets you answer "what happens after the
handshake?" with confidence.

---

## Appendix D — Glossary

| Term | Meaning |
|---|---|
| **CTCP** | Client-To-Client Protocol. Structured messages inside a `PRIVMSG`, delimited by `\x01`. Servers relay them blindly |
| **DCC** | Direct Client-to-Client. A CTCP-negotiated **direct** TCP connection between two clients, bypassing the server |
| **Trailing parameter** | The last IRC parameter, introduced by `:`, running to end of line. The only one that may contain spaces |
| **Prefix** | The `:nick!user@host` (or `:servername`) at the start of a message, identifying its origin |
| **Numeric reply** | A 3-digit server response code, e.g. `001`, `433`, `482` |
| **Registration** | The `PASS` → `NICK` → `USER` handshake. Until it completes, a connection may do almost nothing |
| **Framing** | Reassembling a byte stream into complete `\r\n`-terminated messages. TCP does not preserve message boundaries |
| **Non-blocking** | An fd whose `recv`/`send` return immediately rather than waiting. Set with `fcntl(fd, F_SETFL, O_NONBLOCK)` |
| **`POLLIN` / `POLLOUT`** | Poll events meaning "readable without blocking" / "writable without blocking" |
| **`POLLHUP` / `POLLERR`** | Poll events meaning the peer hung up / an error occurred on the fd |
| **Short write** | `send()` accepting fewer bytes than offered. Normal on non-blocking sockets; a data-loss bug if unhandled |
| **Channel operator** | A user with `+o` on a channel, able to `KICK`, `INVITE`, set `TOPIC` under `+t`, and change modes |
| **Reference client** | The real IRC client you nominate and are evaluated against. Usually irssi |
| **Use-after-free** | Reading memory that has been freed. Frequently appears to work, then does not. See [1.1](#11-the-use-after-free-blocker) |

---

## Sources

- `en.subject.pdf` — ft_irc subject v10.0, in this repository. Every quoted
  requirement above is page-referenced to it.
- **RFC 1459** — Internet Relay Chat Protocol (1993). The message grammar, the
  512-byte limit, the numeric replies.
- **RFC 2812** — Internet Relay Chat: Client Protocol (2000). Updates and
  clarifies RFC 1459's client side.
- **Modern IRC documentation** — <https://modern.ircdocs.horse/> — a maintained,
  readable consolidation of what real servers and clients actually do. The single
  most useful reference for this project.
- **CTCP specification** — <https://modern.ircdocs.horse/ctcp.html>
- **DCC specification** — the original description by Troy Rollo remains the
  canonical text; `modern.ircdocs.horse` covers the parts still in use.
- Your own `srcs/` — the most-cited source in this document.
