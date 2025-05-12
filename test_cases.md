Test:
	PASS mypass
	NICK user
	USER u u u :real

Çıktı:
	:irc.localhost 001 test :Welcome to the IRC server, test!
	:irc.localhost 002 test :Your host is irc.localhost, running version 0.1
	:irc.localhost 003 test :This server was created just now
	:irc.localhost 004 test irc.localhost 0.1 iowghraAsORTVSxNCWqBzvdHtGp

---------------------------------------------------------------------------------

Test:
	PASS mypass
	NICK user
	USER u

Çıktı:
	461 user USER :Not enough parameters

---------------------------------------------------------------------------------

Test:
	PASS mypass
	NICK user (user nickiyle bir client zaten servera bağlıyken)

Çıktı:
	433 user :Nickname is already in use

---------------------------------------------------------------------------------

Test:
	JOIN #test (yeni kurulan bir kanal)

Çıktı:
	:user!u@localhost JOIN :#test
	:irc.localhost 331 user #test :No topic is set
	:irc.localhost 353 user = #test :@user 
	:irc.localhost 366 user #test :End of /NAMES list.

---------------------------------------------------------------------------------

Test:
	MODE #test +k 123 (operator iken)

Çıktı:
	:user!u@localhost MODE #test +k

---------------------------------------------------------------------------------

Test:
	JOIN #test (şifreli kanala şifre girmeden katılmaya çalışmak)

Çıktı:
	:irc.localhost 475 fekiz #test :Cannot join channel (+k)

---------------------------------------------------------------------------------

Test:
	JOIN #test 12 (yanlış şifreyle katılmaya çalışmak)

Çıktı:
	:irc.localhost 475 fekiz #test :Cannot join channel (+k)

---------------------------------------------------------------------------------

Test:
	JOIN #test 123 (doğru şifreyle katılmaya çalışmak)

Çıktı:
	:fekiz!fekiz@localhost JOIN :#test
	:irc.localhost 331 fekiz #test :No topic is set
	:irc.localhost 353 fekiz = #test :@user fekiz 
	:irc.localhost 366 fekiz #test :End of /NAMES list.

---------------------------------------------------------------------------------

Test:
	MODE #genel +l 1 (kanalın üye sayısını 1 yapmak)

Çıktı:
	:user!u@localhost MODE #genel +l 1

---------------------------------------------------------------------------------

Test:
	JOIN #genel (üye limiti 1 olan kanala katılmaya çalışmak)

Çıktı:
	:irc.localhost 471 fekiz #genel :Cannot join channel (+l)

---------------------------------------------------------------------------------

Test:
	MODE #deneme +i (yalnızca davetle üye girişi olan kanal oluşturmak)

Çıktı:
	:user!u@localhost MODE #test +i

---------------------------------------------------------------------------------

Test:
	JOIN #test (yalnızca davetle üye girişi olan kanala katılmaya çalışmak)

Çıktı:
	:irc.localhost 473 xyz #test :Cannot join channel (+i)

---------------------------------------------------------------------------------

Test:
	INVITE xyz #test (invite only kanala birini davet etmek)

Çıktı:
	:irc.localhost 341 user xyz #test

---------------------------------------------------------------------------------

Test:
	MODE #test -i (invite only kanalı herkese açık yapmak)

Çıktı:
	:user!u@localhost MODE #test -i

---------------------------------------------------------------------------------

Test:
	MODE #test -k (şifreli kanalın şifresini kaldırmak)

Çıktı:
	:user!u@localhost MODE #test -k


---------------------------------------------------------------------------------

Test:
	MODE #ytr +l (kişi sayısı girmeden kanala limit koymak)

Çıktı:
	472 user +l :Unknown MODE flag


---------------------------------------------------------------------------------

Test:
	MODE #ytr -l (kişi sayısı limitini kaldırmak)

Çıktı:
	:user!u@localhost MODE #ytr -l


---------------------------------------------------------------------------------

Test:
	MODE #qwe +t (yalnızca operator topic (kanal başlığı) oluşturabilir)

Çıktı:
	:test!test@localhost MODE #qwe +t

---------------------------------------------------------------------------------

Test:
	MODE #qwe -t (yalnızca operatorun topic oluşturmasını kaldır)

Çıktı:
	:test!test@localhost MODE #qwe -t

---------------------------------------------------------------------------------

Test:
	PASS (sunucu şifresi girmeden)

Çıktı:
	:irc.localhost 461 PASS :Not enough parameters

---------------------------------------------------------------------------------

Test:
	PASS mypass (zaten giriş yapmışken tekrar denemek)

Çıktı:
	PASS rejected: Already authenticated. (sunucudaki çıktı)

---------------------------------------------------------------------------------

Test:
	NICK (kullanıcı adı yazmadan denemek)

Çıktı:
	431 :No nickname given

---------------------------------------------------------------------------------

Test:
	USER (herhangi bir giriş yapmadan)

Çıktı:
	461 test USER :Not enough parameters

---------------------------------------------------------------------------------

Test:
	USER u u (eksik parametre ile denemek)

Çıktı:
	461 test USER :Not enough parameters

---------------------------------------------------------------------------------

Test:
	JOIN test (# kullanmadan kanal oluşturmaya ya da katılmaya çalışmak)

Çıktı:
	JOIN: Invalid channel name. (sunucu çıktısı)

---------------------------------------------------------------------------------

Test:
	JOIN #x (sunucuya kayıt olmadan (yalnızca PASS <sifre> girişi yaptıktan sonra) kanala katılmaya çalışmak)

Çıktı:
	:irc.localhost 451 * JOIN :You have not registered

---------------------------------------------------------------------------------

Test:
	MODE #test +x (geçersiz flag kullanmak)

Çıktı:
	472 test +x :Unknown MODE flag

---------------------------------------------------------------------------------

Test:
	TOPIC #test :Bu bir testtir (kanala başlık eklemek)

Çıktı:
	:test!test@localhost TOPIC #test :Bu bir testtir

---------------------------------------------------------------------------------

Test:
	TOPIC #test :deneme (kanalda yalnızca operatorun başlık değiştirmesine izin veriliyorken operator olmayan kullanıcıdan başlık değiştirmeye çalışmak)

Çıktı:
	482 deneme #test :You're not channel operator

---------------------------------------------------------------------------------

Test:
	KICK deneme #qwe (kullanıcıyı kanaldan atmak (irssi'de sorunsuz çalışıyor ama nc'de çıktı farklı))

Çıktı:
	403 test deneme :No such channel

nc'de hatalı kullanım: KICK deneme #qwe

Olması gereken: KICK #kanal kullanici :mesaj

Çıktı: :test!test@localhost KICK #m tester :test

---------------------------------------------------------------------------------

Test:
	INVITE x #kontrol (üyesi olmadığımız kanala birini davet etmeye çalışmak)

Çıktı:
	442 tester #kontrol :You're not on that channel

---------------------------------------------------------------------------------

Test:
	INVITE x #kontrol (operatoru olmadığımız kanalda birini davet etmeye çalışmak)

Çıktı:
	482 deneme #test :You're not channel operator

---------------------------------------------------------------------------------

Test:
	MODE +i #r (sıra farkı (başka bir kullanıcının modunu değiştirmeye çalışmak ))

Çıktı:
	502 tester :Cannot change mode for other users

---------------------------------------------------------------------------------

Test:
	MODE fksjdnfjk #r (var olmayan kullanıcıyı kanala davet etmek)

Çıktı:
	401 tester fdskjhgnfd :No such nick

---------------------------------------------------------------------------------

Test:
	PART #r (kanalı kapatmak)

Çıktı:
	:tester!tester@localhost PART #r

Sunucu çıktısı:
	User tester left channel #r
	Deleted empty channel: #r

---------------------------------------------------------------------------------

Test:
	PART #y (üyesi olmadığın kanalı kapatmaya çalışmak)

Çıktı:
	PART: You are not in channel #p (sunucu çıktısı)

---------------------------------------------------------------------------------

Test:
	QUIT :test (Sunucudan çıkmaya çalışmak)

Çıktı:
	:deneme!deneme@localhost QUIT :test
	:deneme!deneme@localhost QUIT :test
	:deneme!deneme@localhost QUIT :test
	:deneme!deneme@localhost QUIT :test
	ERROR :Closing Link: deneme (irc.localhost) [Client Quit]

Sunucudaki başka bir üyenin terminalindeki çıktı: (Sunucudaki tüm kullanıcılarda çıktı olması gerekiyordu)
	:deneme!deneme@localhost JOIN :#g
	:deneme!deneme@localhost QUIT :test
	:deneme!deneme@localhost QUIT :test
	:deneme!deneme@localhost QUIT :test
	
---------------------------------------------------------------------------------

Test:
	PRIVMSG kullanici :test (olmayan kullanıcıya mesaj göndermeye çalışmak)

Çıktı:
	PRIVMSG: No such channel #j (sunucu çıktısı)

---------------------------------------------------------------------------------

Test:
	PRIVMSG #test :deneme (olmayan kanala mesaj göndermeye çalışmak)

Çıktı:
	PRIVMSG: No such nick ftr (sunucu çıktısı)

---------------------------------------------------------------------------------

Test:
	PRIVMSG #test :deneme (üyesi olmadığımız kanala mesaj göndermeye çalışmak)

Çıktı:
	PRIVMSG: Sender is not in channel #yrt (sunucu çıktısı)

---------------------------------------------------------------------------------

Test:
	NOTICE #test :sa (kanala yayın yapmak)

Çıktı:
	:test!test@localhost NOTICE #u :sa (kanaldaki tüm üyelerdeki çıktı)

---------------------------------------------------------------------------------

Test:
	MODE #u +o kullanici (kanaldaki kullanıcılardan birine owner atamak)

Çıktı:
	:test!test@localhost MODE #u +o tester (kanaldaki tüm kullanıcılardaki çıktı)

---------------------------------------------------------------------------------

Test:
	MODE #u +o frt (olmayan kullanıcıya yetki vermeye çalışmak)

Çıktı:
	401 test frt :No such nick

---------------------------------------------------------------------------------

Test:
	MODE #z +o fekiz (kanalda olmayan kullanıcıya yetki vermeye çalışmak)

Çıktı:
	441 test tester #z :They aren't on that channel

---------------------------------------------------------------------------------

Test:
	MODE #z +o (eksik parametre)

Çıktı:
	472 test +o :Unknown MODE flag

---------------------------------------------------------------------------------

Test:
	TOPIC #k :new (sonradan operator yetkisi alan kullanıcının kanal başlığını değiştirmeye çalışması)

Çıktı:
	:tester!tester@localhost TOPIC #k :new (kanaldaki tüm kullanıcılarda)