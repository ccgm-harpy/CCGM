@echo off
echo Starting webserver...
timeout /t 3 >nul
start web_server.bat
echo Starting Discord bot...
timeout /t 3 >nul
start discord_bot.bat
echo Starting Discord info bot...
timeout /t 3 >nul
start discord_bot_info.bat