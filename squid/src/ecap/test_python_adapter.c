#include "adapter_python_interface.h"
#include <sys/time.h>

struct {
    const char *name, *value, *expected;
} *hhh = NULL, headers[] = {
    {"Server", "AtyponWS/7.1", "AtyponWS/7.1"},
    {"Cache-Control", "private", "private"},
    {"X-XSS-Protection", "1; mode=block", "1; mode=block"},
    {"X-Content-Type-Options", "nosniff", "nosniff"},
    {"Strict-Transport-Security", "max-age=16070400", "max-age=16070400"},
    {"X-Frame-Options", "SAMEORIGIN", "SAMEORIGIN"},
    {"P3P", "CP=\"NOI DSP ADM OUR IND OTC\"", "CP=\"NOI DSP ADM OUR IND OTC\""},
    {"Location", "https://liebertpub.com/?cookieSet=1", "https://liebertpub-com.squid1.example.com/?cookieSet=1"},
    {"Set-Cookie", "I2KBRCK=1; domain=.liebertpub.com; path=/; secure; expires=Thu, 09-Jul-2020 18:13:02 GMT", "I2KBRCK=1; domain=squid1.example.com;path=/; secure; expires=Thu, 09-Jul-2020 18:13:02 GMT"},
    {"Content-Type", "text/html; charset=utf-8", "text/html; charset=utf-8"},
    {"Content-Length", "73", ""},
    {"Cache-Control", "private", "private"},
    {"X-XSS-Protection", "1; mode=block", "1; mode=block"},
    {"X-Content-Type-Options", "nosniff", "nosniff"},
    {"Strict-Transport-Security", "max-age=16070400", "max-age=16070400"},
    {"X-Frame-Options", "SAMEORIGIN", "SAMEORIGIN"},
    {"Location", "https://liebertpub.com/", "https://liebertpub-com.squid1.example.com/"},
    {"Content-Type", "text/html; charset=utf-8", "text/html; charset=utf-8"},
    {"Content-Length", "61", ""},
    {"Server", "AtyponWS/7.1", "AtyponWS/7.1"},
    {"X-XSS-Protection", "1; mode=block", "1; mode=block"},
    {"X-Content-Type-Options", "nosniff", "nosniff"},
    {"Strict-Transport-Security", "max-age=16070400", "max-age=16070400"},
    {"X-Frame-Options", "SAMEORIGIN", "SAMEORIGIN"},
    {"Cache-Control", "no-cache", "no-cache"},
    {"Pragma", "no-cache", "no-cache"},
    {"X-Webstats-RespID", "d913a7abb00b75381b8958c5074caf74", "d913a7abb00b75381b8958c5074caf74"},
    {"Set-Cookie", "SERVER=WZ6myaEXBLEUkiVfXKAZxw==; domain=.liebertpub.com; path=/; secure", "SERVER=WZ6myaEXBLEUkiVfXKAZxw==; domain=squid1.example.com;path=/; secure"},
    {"Set-Cookie", "MAID=+UmureG7rQ9lENHcxBGyPQ==; domain=.liebertpub.com; path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT", "MAID=+UmureG7rQ9lENHcxBGyPQ==; domain=squid1.example.com;path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT"},
    {"Set-Cookie", "MACHINE_LAST_SEEN=2019-07-10T11%3A13%3A03.030-07%3A00; domain=.liebertpub.com; path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT", "MACHINE_LAST_SEEN=2019-07-10T11%3A13%3A03.030-07%3A00; domain=squid1.example.com;path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT"},
    {"Set-Cookie", "JSESSIONID=aaaZtTm03pp3J4DfcAzUw; domain=.liebertpub.com; path=/; secure; HttpOnly", "JSESSIONID=aaaZtTm03pp3J4DfcAzUw; domain=squid1.example.com;path=/; secure; HttpOnly"},
    {"Set-Cookie", "JSESSIONID=aaaZtTm03pp3J4DfcAzUw; path=/; secure; HttpOnly", "JSESSIONID=aaaZtTm03pp3J4DfcAzUw; path=/; secure; HttpOnly"},
    {"Content-Type", "text/html; charset=UTF-8", "text/html; charset=UTF-8"},
    {"Transfer-Encoding", "chunked", "chunked"},
    {"Date", "Wed, 10 Jul 2019 18:13:03 GMT", "Wed, 10 Jul 2019 18:13:03 GMT"},
    {"Vary", "Accept-Encoding", "Accept-Encoding"},
    {"Content-Type", "text/html; charset=UTF-8", "text/html; charset=UTF-8"},
    {"Content-Type", "text/html; charset=UTF-8", "text/html; charset=UTF-8"},
    {"Content-Type", "text/html; charset=UTF-8", "text/html; charset=UTF-8"},
    {NULL, NULL, NULL}
};

struct {
    const char *uri;
    int request_flag;
    int n_header;
    const char *vb;
    const char *ab;
} *ptr=NULL, cases[] = {
    {"none", 0, 12, "<html><body><a href=\"http://www.medicinanet.com.br/videos/aulas/663/estado_de_mal_asmatico_no_pronto_socorro_pediatrico.htm\">estado de mal asmatico no pronto socorro pediatrico</a></body></html>", "<html><body><a href=\"http://www-medicinanet-com-br.squid1.example.com/videos/aulas/663/estado_de_mal_asmatico_no_pronto_socorro_pediatrico.htm\">estado de mal asmatico no pronto socorro pediatrico</a></body></html>"},
    {"uno", 0, 12, "<html><body><img src=\"http://www.medicinanet.com.br/img/logo_medicinanet.gif\"></body></html>", "<html><body><img src=\"http://www-medicinanet-com-br.squid1.example.com/img/logo_medicinanet.gif\"></body></html>"},
    {"duo", 0, 12, "<a href=\"http://www.liebertpub.com\">http://www.liebertpub.com http://www.medicinanet.com.br</a>", "<a href=\"http://www-liebertpub-com.squid1.example.com\">http://www-liebertpub-com.squid1.example.com http://www-medicinanet-com-br.squid1.example.com</a>"},
    {"medicinanet", 0, 1, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"> \
<html xmlns=\"http://www.w3.org/1999/xhtml\"><head> \
 \
<style type=\"text/css\"> \
    .pubdiv{ padding: 2px; spacing: 1px; border: 0px; line-height: 15px; } \
</style> \
 \
 \
<script type=\"text/javascript\" src=\"MedicinaNET_files/Naja_Systemv0.ashx\"></script><style>@media print {#ghostery-purple-box {display:none !important}}</style> \
<script type=\"text/javascript\" src=\"MedicinaNET_files/Naja_Publisher.ashx\"></script> \
 \
		   <title>MedicinaNET</title>  \
		    <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">   \
			<meta http-equiv=\"Expires\" content=\"-1\">  \
			<meta http-equiv=\"pragma\" content=\"no-cache\"> \
			<meta http-equiv=\"Content-Language\" content=\"pt-br\">  \
			<meta name=\"GOOGLEBOT\" content=\"NOARCHIVE\"> \
			<meta name=\"robots\" content=\"all\">  \
			<meta name=\"verify-v1\" content=\"C1yYFNmyoMwVtMVeW0Zeb6UDSQwFrzXB536yEFZUn78=\">   \
			<meta http-equiv=\"keywords\" content=\"\">  <meta http-equiv=\"description\" content=\"\">   \
			<meta http-equiv=\"X-UA-Compatible\" content=\"IE=EmulateIE8\">    \
			<link rel=\"alternate\" href=\"http://www.medicinanet.com.br/medicinanet.rss\" title=\"MedicinaNet - RSS - Brasil\" type=\"application/rss+xml\">   \
			<meta http-equiv=\"Content-Style-Type\" content=\"text/css\">  \
			 <link rel=\"stylesheet\" type=\"text/css\" href=\"MedicinaNET_files/reset.css\">  \
			  <link rel=\"stylesheet\" type=\"text/css\" href=\"MedicinaNET_files/estilo.css\"> \
			    <link rel=\"stylesheet\" type=\"text/css\" href=\"MedicinaNET_files/estrutura.css\">  \
				 <link rel=\"stylesheet\" type=\"text/css\" href=\"MedicinaNET_files/dropdown.css\">  \
				  <link rel=\"stylesheet\" type=\"text/css\" href=\"MedicinaNET_files/inicial.css\">  \
				  <script type=\"text/javascript\" src=\"MedicinaNET_files/tinyslider.js\"></script>   \
				  <script src=\"MedicinaNET_files/global.js\" type=\"text/javascript\"></script>    \
 \
				  <script async=\"\" src=\"MedicinaNET_files/adsbygoogle.js\"></script> \
<script> \
     (adsbygoogle = window.adsbygoogle || []).push({ \
          google_ad_client: \"ca-pub-2078506165513007\", \
          enable_page_level_ads: true \
     }); \
</script> \
				    <!-- Favicon -->   <link rel=\"shortcut icon\" href=\"http://www.medicinanet.com.br/img/favicon.ico\">  \
					  <link rel=\"icon\" href=\"http://www.medicinanet.com.br/img/favicon.ico\" type=\"image/x-icon\">       \
					     \
<script type=\"text/javascript\">var varPerfilN=\"\";</script> \
					   <!-- IPAD=|200.130.19.88|  --><!-- IPAD=|200.130.19.88| (-1) {200.130.19.88}Count=  --><!-- Corporate: Null --> <link rel=\"alternate\" media=\"only screen and (max-width: 640px)\" href=\"https://www.medicinanet.com.br/m/home.htm\"> \
 \
<script language=\"javascript\" type=\"text/javascript\"> \
$namespace('System.Data'); \
$namespace('System.Security');  \
$namespace('System.Controls.IncrementalSearch'); \
$namespace('System.Controls.ToolTip'); \
</script><script language=\"javascript\" type=\"text/javascript\" src=\"MedicinaNET_files/Naja_System_i5v0.ashx\"></script> \
<script language=\"javascript\" type=\"text/javascript\" src=\"MedicinaNET_files/Naja_System_Azv0.ashx\"></script> \
 \
<style type=\"text/css\"> \
    #divbannersh { \
        position: absolute;  \
        display:none; \
        width:736px; \
        top: 50%; \
        width: 100%;  \
        height: 320px;  \
        margin-top: -125px; \
        color: #FFF; \
        text-align: center;  \
        z-index: 1000;  \
    } \
    #fechar { margin: 5px; font-size: 12px; } \
  </style> \
</head><div id=\"ghostery-purple-box\" class=\"ghostery-bottom ghostery-right ghostery-collapsed\"><div id=\"ghostery-box\"><div id=\"ghostery-count\" style=\"background: rgba(0, 0, 0, 0) none repeat scroll 0% 0%; color: rgb(255, 255, 255);\">3</div><div id=\"ghostery-pb-icons-container\"><span id=\"ghostery-breaking-tracker\" class=\"ghostery-pb-tracker\" title=\"Broken Page Trackers\" style=\"background: rgba(0, 0, 0, 0) url(&quot;data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiIHN0YW5kYWxvbmU9Im5vIj8+DQo8c3ZnIHdpZHRoPSIxOHB4IiBoZWlnaHQ9IjE4cHgiIHZpZXdCb3g9IjAgMCAxOCAxOCIgdmVyc2lvbj0iMS4xIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHhtbG5zOnhsaW5rPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5L3hsaW5rIj4NCiAgICA8IS0tIEdlbmVyYXRvcjogU2tldGNoIDQwICgzMzc2MikgLSBodHRwOi8vd3d3LmJvaGVtaWFuY29kaW5nLmNvbS9za2V0Y2ggLS0+DQogICAgPHRpdGxlPmJyZWFraW5ncGFnZTwvdGl0bGU+DQogICAgPGRlc2M+Q3JlYXRlZCB3aXRoIFNrZXRjaC48L2Rlc2M+DQogICAgPGRlZnM+PC9kZWZzPg0KICAgIDxnIGlkPSJQdXJwbGUtQm94IiBzdHJva2U9Im5vbmUiIHN0cm9rZS13aWR0aD0iMSIgZmlsbD0ibm9uZSIgZmlsbC1ydWxlPSJldmVub2RkIj4NCiAgICAgICAgPGcgdHJhbnNmb3JtPSJ0cmFuc2xhdGUoLTQxNi4wMDAwMDAsIC00NTMuMDAwMDAwKSIgaWQ9ImJhbSEtYnJlYWtpbmctdGhlLXBhZ2UtY29weS0yIiBmaWxsPSIjRkNCQTMzIj4NCiAgICAgICAgICAgIDxnIHRyYW5zZm9ybT0idHJhbnNsYXRlKDQxNi4wMDAwMDAsIDQ1My4wMDAwMDApIj4NCiAgICAgICAgICAgICAgICA8cGF0aCBkPSJNOSwwLjE5NTY1MjE3NCBDNC4xNDQzNjAyNSwwLjE5NTY1MjE3NCAwLjE5NTY1MjE3NCw0LjE0NDM2MDI1IDAuMTk1NjUyMTc0LDkgQzAuMTk1NjUyMTc0LDEzLjg1NTYzOTggNC4xNDQzNjAyNSwxNy44MDQzNDc4IDksMTcuODA0MzQ3OCBDMTMuODU1NjM5OCwxNy44MDQzNDc4IDE3LjgwNDM0NzgsMTMuODU1NjM5OCAxNy44MDQzNDc4LDkgQzE3LjgwNDM0NzgsNC4xNDQzNjAyNSAxMy44NTU2Mzk4LDAuMTk1NjUyMTc0IDksMC4xOTU2NTIxNzQgWiBNMTEuNDg1NTg5OSwxMy40MTA0NDQxIEwxMS4wNzcwNzk4LDEzLjAyMDY3NjggTDEyLjEwMDQ3MTEsMTIuMjE2OTU3OSBMMTEuMDQ2MjQ1MSwxMi4yMTY5NTc5IEwxMS4yMzQ0NzgxLDEwLjg3MDcwODcgTDkuODAzMTgxNDIsMTEuNzk1NzUxMiBMOS40MDMzMzczNCw5LjM0NTA5MzkyIEw4LjY5NDc0MjY5LDExLjA4NjU1MTkgTDcuMzI1NzIwMDksMTAuMTcwOTgxNSBMNy43NTI1Njk3NywxMS45Mjk1NyBMNi41NTQyNDY3MywxMi4zMTE0Nzc1IEw3Ljg4MjM1Nzg3LDEzLjQxMDQ0NDEgTDExLjQ4NTU4OTksMTMuNDEwNDQ0MSBaIE02LjcxNTY3NTcyLDEzLjQxMDQ0NDEgTDUuMDI4NjMxOTcsMTIuMDA2NzU3NiBMNi44Njg0Mzg3MywxMS40MzE5ODE4IEw2LjE2Mzg3NDc3LDguNDg4NTczMDkgTDguMzQ5MzEyODgsOS45NTk5NzUxMiBMOS43MDQwMjY1NCw2LjYxMjQ5MDE1IEwxMC4zNTAzNDcxLDEwLjU1NjcxODIgTDEyLjE5NDk5MDcsOS4zNzY1MzMyOCBMMTEuODk4OTM2OCwxMS40NzY5MjM5IEwxNC4yNjI5MzQzLDExLjQ3NjkyMzkgTDEyLjIxMjkyNzIsMTMuMDc4OTIwMiBMMTIuNTY3MjI0NSwxMy40MTA0NDQxIEwxNS4zMzEyNjc3LDEzLjQxMDQ0NDEgTDE0LjQ3Mzk0MDcsMTIuNTk4NjYzOSBMMTcuMjA3MzUwNiwxMC40NjY4MzM5IEwxMy4wNjA3ODIxLDEwLjQ2NjgzMzkgTDEzLjQ5NjI5NzcsNy4zNDg2OTUgTDExLjA5OTg1MzIsOC44Nzg5NDUwNSBMMTAuMTIxMjAyNiwyLjg5Mjc3MTMgTDcuODc3NzIyNTgsOC40MjU0OTI4NSBMNC41NzA1NDQ0Nyw2LjIwMzk4MDEgTDUuNjY1NDgwNDEsMTAuNzUwMzkyNyBMMi45NTEwMTQ3MiwxMS41OTgyNDc2IEw1LjEzNjQ1MjgzLDEzLjQxMDQ0NDEgTDYuNzE1Njc1NzIsMTMuNDEwNDQ0MSBaIiBpZD0iYnJlYWtpbmdwYWdlIj48L3BhdGg+DQogICAgICAgICAgICA8L2c+DQogICAgICAgIDwvZz4NCiAgICA8L2c+DQo8L3N2Zz4=&quot;) repeat scroll 0% 0%; opacity: 0.5;\"></span><span id=\"ghostery-slow-tracker\" class=\"ghostery-pb-tracker\" title=\"Slow Trackers\" style=\"background: rgba(0, 0, 0, 0) url(&quot;data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiIHN0YW5kYWxvbmU9Im5vIj8+DQo8c3ZnIHdpZHRoPSIxN3B4IiBoZWlnaHQ9IjE3cHgiIHZpZXdCb3g9IjAgMCAxNyAxNyIgdmVyc2lvbj0iMS4xIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHhtbG5zOnhsaW5rPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5L3hsaW5rIj4NCiAgICA8IS0tIEdlbmVyYXRvcjogU2tldGNoIDQwICgzMzc2MikgLSBodHRwOi8vd3d3LmJvaGVtaWFuY29kaW5nLmNvbS9za2V0Y2ggLS0+DQogICAgPHRpdGxlPnNsb3d0cmFja2VyczwvdGl0bGU+DQogICAgPGRlc2M+Q3JlYXRlZCB3aXRoIFNrZXRjaC48L2Rlc2M+DQogICAgPGRlZnM+PC9kZWZzPg0KICAgIDxnIGlkPSJQdXJwbGUtQm94IiBzdHJva2U9Im5vbmUiIHN0cm9rZS13aWR0aD0iMSIgZmlsbD0ibm9uZSIgZmlsbC1ydWxlPSJldmVub2RkIj4NCiAgICAgICAgPGcgdHJhbnNmb3JtPSJ0cmFuc2xhdGUoLTM5NS4wMDAwMDAsIC00NTQuMDAwMDAwKSIgaWQ9InNsb3d0cmFja2VycyIgZmlsbD0iI0ZDQkEzMyI+DQogICAgICAgICAgICA8cGF0aCBkPSJNNDAzLjUsNDU0IEMzOTguODEyMjEsNDU0IDM5NSw0NTcuODEyMjEgMzk1LDQ2Mi41IEMzOTUsNDY3LjE4Nzc5IDM5OC44MTIyMSw0NzEgNDAzLjUsNDcxIEM0MDguMTg3NzksNDcxIDQxMiw0NjcuMTg3NzkgNDEyLDQ2Mi41IEM0MTIsNDU3LjgxMjIxIDQwOC4xODc3OSw0NTQgNDAzLjUsNDU0IFogTTQwOS42MDk1ODQsNDY1LjE3ODY1NCBDNDA5LjUzMDI1OSw0NjUuMTU0MDkgNDA4LjY3NzI4Myw0NjQuNzQ2NDIgNDA3LjU2MTA5MSw0NjQuMzYyNjM3IEM0MDguNDg0Mzc4LDQ2My43NDU2MSA0MDkuMDk0NDE4LDQ2Mi42OTM2NDUgNDA5LjA5NDQxOCw0NjEuNTAxNzMzIEM0MDkuMDk0NDE4LDQ1OS42MDU1ODEgNDA3LjU1MTQwMSw0NTguMDYyMzM4IDQwNS42NTUyNDksNDU4LjA2MjMzOCBDNDAzLjc1OTA5Nyw0NTguMDYyMzM4IDQwMi4yMTU4NTQsNDU5LjYwNTU4MSA0MDIuMjE1ODU0LDQ2MS41MDE3MzMgQzQwMi4yMTU4NTQsNDYyLjA0OTM1IDQwMi4zNDUyMDgsNDYyLjU2Njc2OSA0MDIuNTczMjY5LDQ2My4wMjY0OTcgQzQwMi43ODgwMzQsNDYzLjA2ODYzOCA0MDMuMzQ0NDQsNDYzLjE3NTIzMiA0MDQuMjIzNzgyLDQ2My4zMjM5NjggQzQwNS4yMDQ1MzUsNDYzLjQ5MDI4MSA0MDUuODUyNDM2LDQ2My4zNTY0MTkgNDA2LjM5MTAzOSw0NjIuODc2NjM0IEM0MDYuNzI4MTcyLDQ2Mi41NzY0NTkgNDA2LjkyODA2NCw0NjIuMTYzNjA2IDQwNi45NTM1MjksNDYxLjcxMzc5NCBDNDA2Ljk4MDEyMSw0NjEuMjYzOTgxIDQwNi44Mjk1ODMsNDYwLjgyOTk0NCA0MDYuNTI5NDA4LDQ2MC40OTM3MTIgQzQwNi4wNDY5MTksNDU5Ljk1MjQwNSA0MDUuMjE1MTI3LDQ1OS45MDM5NTMgNDA0LjY3MjY5Myw0NjAuMzg1NTQxIEM0MDQuMjM5NTU3LDQ2MC43NzExMjYgNDA0LjE4NTAyMSw0NjEuNDQ0NDkyIDQwNC41NTIxMjcsNDYxLjg1NzM0NiBDNDA0Ljg0MDEzMyw0NjIuMTgwNTA3IDQwNS4zNjk5NDcsNDYyLjIxNzQ2NiA0MDUuNjg2Nzk5LDQ2MS45MzU3NyBDNDA1LjgwMzk4NCw0NjEuODMxODggNDA1Ljg3MzM5NCw0NjEuNjkyODM1IDQwNS44ODA2MDYsNDYxLjU0NDc3NiBDNDA1Ljg4NjY5LDQ2MS40MjQyMSA0MDUuODUwNjMzLDQ2MS4zMTA2MyA0MDUuNzgwOTk4LDQ2MS4yMzQwMDkgQzQwNS43MTg1NzQsNDYxLjE2NTUgNDA1LjYxOTE5Miw0NjEuMTI3NjQxIDQwNS41MTY4OCw0NjEuMTI4NTQyIEM0MDUuNDI5ODkyLDQ2MS4xMzEwMjEgNDA1LjMxNzIxNCw0NjEuMTY1NSA0MDUuMjQ0MTk4LDQ2MS4yMzc2MTUgQzQwNS4yMjYzOTUsNDYxLjI1NDI5MSA0MDUuMjA0NTM1LDQ2MS4yNjQ4ODMgNDA1LjE3OTc0Niw0NjEuMjY0ODgzIEM0MDUuMTI2MTExLDQ2MS4yNjQ4ODMgNDA1LjA4MzA2OCw0NjEuMjE2NDMxIDQwNS4wODMwNjgsNDYxLjE1NzYxMyBDNDA1LjA4MzA2OCw0NjEuMTIxMzMxIDQwNS4wOTc5NDEsNDYxLjA5NDk2NCA0MDUuMTE2NDIxLDQ2MS4wNjk0OTggQzQwNS4yMjYzOTUsNDYwLjkxODk2IDQwNS4zODE0NCw0NjAuODMxNzQ3IDQwNS41MzUzNTksNDYwLjgxODY3NiBDNDA1Ljc0NDAzOSw0NjAuODAxMDk5IDQwNS45MTMwNTcsNDYwLjg2MDgxOCA0MDYuMDQ2OTE5LDQ2MS4wMDc3NTEgQzQwNi4xNzk4NzksNDYxLjE1NDAwNyA0MDYuMjQ5Mjg5LDQ2MS4zNDg0OSA0MDYuMjM3MTIsNDYxLjU2MzI1NSBDNDA2LjIyMzgyNCw0NjEuODA2MTkgNDA2LjExMjk0OCw0NjIuMDMyNDQ4IDQwNS45MjM2NDksNDYyLjIwMDU2NCBDNDA1LjQ1NzE2LDQ2Mi42MTYxMjIgNDA0LjcwNzE3Myw0NjIuNTY2MDkzIDQwNC4yODU1Myw0NjIuMDkzMjk0IEM0MDMuNzg5NzQ1LDQ2MS41MzU5ODcgNDAzLjg1ODQ3OSw0NjAuNjMyMDgxIDQwNC40MzUxNjcsNDYwLjExNzgxNyBDNDA1LjEyMzQwNyw0NTkuNTA1Mjk3IDQwNi4xODI1ODQsNDU5LjU2NjgyIDQwNi43OTQyMDIsNDYwLjI1NTI4NCBDNDA3LjE1NzAyNiw0NjAuNjYyNzMgNDA3LjM0MDAxNiw0NjEuMTg4MjYyIDQwNy4zMDg0NjYsNDYxLjczMzE3NCBDNDA3LjI3NjY5MSw0NjIuMjc4OTg4IDQwNy4wMzQ2NTgsNDYyLjc3OTA1NSA0MDYuNjI2OTg3LDQ2My4xNDE2NTQgQzQwNi4xNjgzODYsNDYzLjU1MDIyNiA0MDUuNjMyMjYyLDQ2My43NDY1MTIgNDA0Ljk0NjUwMiw0NjMuNzQ2NTEyIEM0MDQuNzA1MzcsNDYzLjc0NjUxMiA0MDQuNDQ0ODU3LDQ2My43MjE3MjIgNDA0LjE2MzE2Miw0NjMuNjc0Mzk3IEM0MDMuMTkyMDk5LDQ2My41MDk2NjIgNDAyLjE1NTAwNyw0NjMuMzI0ODY5IDQwMi4wMTU5NjIsNDYzLjMwNTQ4OCBDNDAxLjMxNzEzMSw0NjMuMjEyMTkxIDQwMC43MzYxNjEsNDYyLjczNzU4OSA0MDAuNzE3NjgyLDQ2Mi4wMzk2NTkgTDQwMC44OTQ1ODcsNDU4Ljk4NzY1MyBDNDAwLjg5NDU4Nyw0NTguNzkxMzY3IDQwMC43MzUyNiw0NTguNjMxMTM4IDQwMC41MzgwNzIsNDU4LjYzMTEzOCBDNDAwLjM0MDg4NSw0NTguNjMxMTM4IDQwMC4xODE1NTgsNDU4Ljc5MDQ2NSA0MDAuMTgxNTU4LDQ1OC45ODc2NTMgQzQwMC4xODE1NTgsNDU4Ljk4NzY1MyA0MDAuMjg1NDQ3LDQ2MC44NDE0MzcgNDAwLjI5NzYxNyw0NjEuMDc1NTgzIEM0MDAuMzIwNjAzLDQ2MS41MjAyMTIgMzk5LjkxMTEzLDQ2MS44NzY3MjYgMzk5LjQ2MDQxNiw0NjEuODc2NzI2IEMzOTguOTk4NDM1LDQ2MS44NzY3MjYgMzk4LjU4NzE1OSw0NjEuNTAwODMxIDM5OC42MjM0NDEsNDYxLjAzOTUyNiBDMzk4LjY0MzQ5OCw0NjAuNzg0MTk3IDM5OC42NjQ2ODIsNDYwLjUyMDA3OSAzOTguNjg1ODY1LDQ2MC4yNzI4NjIgTDM5OC43NTk3ODIsNDU5LjAwOTUxMiBDMzk4Ljc1OTc4Miw0NTguODEzMjI2IDM5OC42MDA0NTUsNDU4LjY1Mjk5OCAzOTguNDAzMjY4LDQ1OC42NTI5OTggQzM5OC4yMDYwOCw0NTguNjUyOTk4IDM5OC4wNDY3NTMsNDU4LjgxMjMyNSAzOTguMDQ2NzUzLDQ1OS4wMDk1MTIgTDM5OC4yMjAyNzgsNDYxLjk5OTk5NyBMMzk4LjIyMDI3OCw0NjIuMDA1MTggQzM5OC4yMjAyNzgsNDY0LjA5NzYxNyAzOTkuNDE3MzczLDQ2NS44MDI4OTIgNDAxLjUxMDcxMiw0NjUuODAxMDg5IEM0MDMuNjIyNTMxLDQ2NS43OTgzODUgNDA5LjYwODY4Myw0NjUuODAxMDg5IDQwOS42MDg2ODMsNDY1LjgwMTA4OSBDNDA5Ljc4MTA4MSw0NjUuODAxMDg5IDQwOS45MjAzNTEsNDY1LjY2MTE0MyA0MDkuOTIwMzUxLDQ2NS40ODk0MjEgQzQwOS45MjAzNTEsNDY1LjMxNzAyMyA0MDkuNzczMTkzLDQ2NS4yMzA3MTEgNDA5LjYwOTU4NCw0NjUuMTc4NjU0IFoiPjwvcGF0aD4NCiAgICAgICAgPC9nPg0KICAgIDwvZz4NCjwvc3ZnPg==&quot;) repeat scroll 0% 0%; opacity: 0.5;\"></span><span id=\"ghostery-non-secure-tracker\" class=\"ghostery-pb-tracker\" title=\"Non-secure Trackers\" style=\"background: rgba(0, 0, 0, 0) url(&quot;data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiIHN0YW5kYWxvbmU9Im5vIj8+DQo8c3ZnIHdpZHRoPSIxOHB4IiBoZWlnaHQ9IjE4cHgiIHZpZXdCb3g9IjAgMCAxOCAxOCIgdmVyc2lvbj0iMS4xIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHhtbG5zOnhsaW5rPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5L3hsaW5rIj4NCiAgICA8IS0tIEdlbmVyYXRvcjogU2tldGNoIDQwICgzMzc2MikgLSBodHRwOi8vd3d3LmJvaGVtaWFuY29kaW5nLmNvbS9za2V0Y2ggLS0+DQogICAgPHRpdGxlPndhcm5pbmc8L3RpdGxlPg0KICAgIDxkZXNjPkNyZWF0ZWQgd2l0aCBTa2V0Y2guPC9kZXNjPg0KICAgIDxkZWZzPjwvZGVmcz4NCiAgICA8ZyBpZD0iUHVycGxlLUJveCIgc3Ryb2tlPSJub25lIiBzdHJva2Utd2lkdGg9IjEiIGZpbGw9Im5vbmUiIGZpbGwtcnVsZT0iZXZlbm9kZCI+DQogICAgICAgIDxnIHRyYW5zZm9ybT0idHJhbnNsYXRlKC0zNzMuMDAwMDAwLCAtNDUzLjAwMDAwMCkiIGlkPSJ3YXJuaW5nIiBmaWxsPSIjRkVCMDMyIj4NCiAgICAgICAgICAgIDxnIHRyYW5zZm9ybT0idHJhbnNsYXRlKDM3My4wMDAwMDAsIDQ1My4wMDAwMDApIj4NCiAgICAgICAgICAgICAgICA8cGF0aCBkPSJNOSwwLjYzMDQzNDc4MyBDNC4zODQxNDQ5MywwLjYzMDQzNDc4MyAwLjYzMDQzNDc4Myw0LjM4NDE0NDkzIDAuNjMwNDM0NzgzLDkgQzAuNjMwNDM0NzgzLDEzLjYxNTg1NTEgNC4zODQxNDQ5MywxNy4zNjk1NjUyIDksMTcuMzY5NTY1MiBDMTMuNjE1ODU1MSwxNy4zNjk1NjUyIDE3LjM2OTU2NTIsMTMuNjE1ODU1MSAxNy4zNjk1NjUyLDkgQzE3LjM2OTU2NTIsNC4zODQxNDQ5MyAxMy42MTU4NTUxLDAuNjMwNDM0NzgzIDksMC42MzA0MzQ3ODMgWiBNNC42NDI5MjgxMSwxMS43ODk4NTUxIEM1LjI1MDQxMTY1LDExLjc4OTg1NTEgNS43NTY5NTIzNCwxMS4zNjA3NTY3IDUuODc4NzE2OTMsMTAuODgxMzY5NSBDNi4wMDA0ODE1MiwxMS4zNjEyNDM3IDYuNTA3MDIyMjIsMTEuNzIzNzM2OSA3LjExNDM4NCwxMS43MjM3MzY5IEM3LjcyNDE4MTA2LDExLjcyMzczNjkgOC4yMzI2Njk5OSwxMS4zNjUwMTg0IDguMzUxNTEyMjMsMTAuODgyNzA4OSBDOC40NzA5NjMzLDExLjM2NTAxODQgOC45Nzk0NTIyMywxMS43MzY1MjIyIDkuNTg4NzYyMjQsMTEuNzM2NTIyMiBDMTAuMTk0NjYyOCwxMS43MzY1MjIyIDEwLjcwMTIwMzUsMTEuMzk0OTcyNSAxMC44MjM0NTUyLDEwLjkxNjU1OTQgQzEwLjk0NTcwNjgsMTEuMzk0OTcyNSAxMS40NTIyNDc1LDExLjc4OTM2OCAxMi4wNTgyNjk5LDExLjc4OTM2OCBDMTIuMzUzNjcwOCwxMS43ODkzNjggMTIuNjI0NzE4OCwxMS42OTkzODQgMTIuODM5NzU1LDExLjU1OTU5ODIgQzExLjAwOTUxMTUsOC43MTgwOTk3NSAxMi4xNDUzMzE2LDQuMTM3NjgxMTYgMTIuMTQ1MzMxNiw0LjEzNzY4MTE2IEM2Ljk0NjQ3MDYzLDUuMjMxNjE0MjQgNC42NjU4MTk4NSwxMC4xMDAzNzE0IDQuMDU3OTcxMDEsMTEuNjY2MTQyMiBDNC4yMzI5NDY3MywxMS43NDMyMTkyIDQuNDMxNzg4MzEsMTEuNzg5ODU1MSA0LjY0MjkyODExLDExLjc4OTg1NTEgWiIgaWQ9Indhcm5pbmd0cmFja2VycyI+PC9wYXRoPg0KICAgICAgICAgICAgPC9nPg0KICAgICAgICA8L2c+DQogICAgPC9nPg0KPC9zdmc+&quot;) repeat scroll 0% 0%; opacity: 0.5;\"></span></div><div id=\"ghostery-title\">Trackers</div><div id=\"ghostery-minimize\"><span id=\"ghostery-minimize-icon\"></span></div><span id=\"ghostery-close\" style=\"background: rgba(0, 0, 0, 0) url(&quot;data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiIHN0YW5kYWxvbmU9Im5vIj8+DQo8c3ZnIHdpZHRoPSIxNXB4IiBoZWlnaHQ9IjE1cHgiIHZpZXdCb3g9IjAgMCAxNSAxNSIgdmVyc2lvbj0iMS4xIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHhtbG5zOnhsaW5rPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5L3hsaW5rIj4NCiAgICA8IS0tIEdlbmVyYXRvcjogU2tldGNoIDMuNy4yICgyODI3NikgLSBodHRwOi8vd3d3LmJvaGVtaWFuY29kaW5nLmNvbS9za2V0Y2ggLS0+DQogICAgPHRpdGxlPmNvbGxhcHNlIGNvcHkgMjwvdGl0bGU+DQogICAgPGRlc2M+Q3JlYXRlZCB3aXRoIFNrZXRjaC48L2Rlc2M+DQogICAgPGRlZnM+PC9kZWZzPg0KICAgIDxnIGlkPSJQdXJwbGUtQm94IiBzdHJva2U9Im5vbmUiIHN0cm9rZS13aWR0aD0iMSIgZmlsbD0ibm9uZSIgZmlsbC1ydWxlPSJldmVub2RkIj4NCiAgICAgICAgPGcgaWQ9ImNvbGxhcHNlLWNvcHktMiI+DQogICAgICAgICAgICA8Y2lyY2xlIGlkPSJPdmFsLTMxNSIgZmlsbC1vcGFjaXR5PSIwLjI3MDE1Mzk4NiIgZmlsbD0iI0Q4RDhEOCIgY3g9IjcuNSIgY3k9IjcuNSIgcj0iNy41Ij48L2NpcmNsZT4NCiAgICAgICAgICAgIDxwYXRoIGQ9Ik00LjM2LDQuMzYgTDEwLjU3NDU2MzQsMTAuNTc0NTYzNCIgaWQ9IkxpbmUiIHN0cm9rZT0iI0ZGRkZGRiIgc3Ryb2tlLWxpbmVjYXA9InNxdWFyZSI+PC9wYXRoPg0KICAgICAgICAgICAgPHBhdGggZD0iTTQuMzYsNC4zNiBMMTAuNTc0NTYzNCwxMC41NzQ1NjM0IiBpZD0iTGluZS1Db3B5IiBzdHJva2U9IiNGRkZGRkYiIHN0cm9rZS1saW5lY2FwPSJzcXVhcmUiIHRyYW5zZm9ybT0idHJhbnNsYXRlKDcuNjAwMDAwLCA3LjYwMDAwMCkgc2NhbGUoLTEsIDEpIHRyYW5zbGF0ZSgtNy42MDAwMDAsIC03LjYwMDAwMCkgIj48L3BhdGg+DQogICAgICAgIDwvZz4NCiAgICA8L2c+DQo8L3N2Zz4=&quot;) repeat scroll 0% 0%;\"></span></div><div id=\"ghostery-pb-background\"><div id=\"ghostery-trackerList\"><div class=\"ghostery-trackerContainer ghostery-tracker-disabled\" category=\"advertising\"><span id=\"ghostery-no-tracker\" class=\"ghostery-pb-tracker-list\"></span><span class=\"ghostery-tracker\">Google Adsense</span></div><div class=\"ghostery-trackerContainer ghostery-tracker-disabled\" category=\"site_analytics\"><span id=\"ghostery-no-tracker\" class=\"ghostery-pb-tracker-list\"></span><span class=\"ghostery-tracker\">Google Analytics</span></div><div class=\"ghostery-trackerContainer ghostery-tracker-disabled\" category=\"advertising\"><span id=\"ghostery-no-tracker\" class=\"ghostery-pb-tracker-list\"></span><span class=\"ghostery-tracker\">Teads</span></div></div></div></div><body onload=\"init();\"><div id=\"divbannersh\">  \
<div id=\"fechar\" style=\"width: 907px;\" align=\"right\"><a href=\"javascript:fecharBanner();\">FECHAR</a></div>  \
 \
<iframe src=\"MedicinaNET_files/imgbanner.jpg\" width=\"737\" height=\"315px\"></iframe> \
</div> \
<script type=\"text/javascript\" src=\"MedicinaNET_files/bannerflutuante.asc\"></script> \
 \
<script language=\"javascript\" type=\"text/javascript\"> \
    window.ServerVariables = { \
        codVideo: '', \
        codConteudo: '', \
        menu: '', \
        topico: '',    \
        pesquisa: '', \
        pesquisaTitle: '',    \
        pageName: 'home', \
        isCorporativo: false,   \
        usuarioLogado: false, \
        emailUsr: '', \
        renovaAssinatura: '-1', \
        codPlano: 0, \
        IPAddressUser: '200.130.19.88' \
              ,CodTipo: '', \
        Nanda: '' \
       \
    }; \
 \
 \
</script> \
<script language=\"javascript\" type=\"text/javascript\" src=\"MedicinaNET_files/scripts.aspm\"></script> \
 \
 \
<script language=\"javascript\" type=\"text/javascript\"> \
if (ServerVariables.usuarioLogado) { \
} else { \
if (varPerfilN=='medico') {     \
if ($('banners_internas')) {$('banners_internas').show(); } \
// if ($('bannerTeste')) {$('bannerTeste').hide(); } \
if ($('bannerNews')) {$('bannerNews').show(); } \
if ($('bannerAssine')) {$('bannerAssine').show(); } \
if ($('box-twitter')) { $('box-twitter').show(); } \
if ($('facebook-box')) { $('facebook-box').show(); } \
if ($('adwords')) { $('adwords').hide(); } \
if ($('adwordstopo')) { $('adwordstopo').hide(); } \
if ($('adwordstopo2')) { $('adwordstopo2').hide(); } \
 \
} \
 \
if (varPerfilN=='publico') {     \
if ($('banners_internas')) {$('banners_internas').show(); } \
if ($('bannerNews')) {$('bannerNews').show(); } \
if ($('bannerAssine')) {$('bannerAssine').show(); } \
if ($('box-twitter')) { $('box-twitter').show(); } \
if ($('facebook-box')) { $('facebook-box').show(); } \
if ($('adwords')) { $('adwords').show(); } \
if ($('adwordstopo')) { $('adwordstopo').show(); } \
if ($('adwordstopo2')) { $('adwordstopo2').show(); } \
 \
} \
if (varPerfilN=='profissional') {     \
if ($('banners_internas')) {$('banners_internas').show(); } \
// if ($('bannerTeste')) {$('bannerTeste').hide(); } \
if ($('bannerNews')) {$('bannerNews').show(); } \
if ($('bannerAssine')) {$('bannerAssine').show(); } \
if ($('box-twitter')) { $('box-twitter').show(); } \
if ($('facebook-box')) { $('facebook-box').show(); } \
if ($('adwords')) { $('adwords').show(); } \
if ($('adwordstopo')) { $('adwordstopo').show(); } \
if ($('adwordstopo2')) { $('adwordstopo2').show(); } \
 \
} \
if (varPerfilN=='estudante') {     \
if ($('banners_internas')) {$('banners_internas').show(); } \
//if ($('bannerTeste')) {$('bannerTeste').hide(); } \
if ($('bannerNews')) {$('bannerNews').show(); } \
if ($('bannerAssine')) {$('bannerAssine').show(); } \
if ($('adwords')) { $('adwords').show(); } \
if ($('adwordstopo')) { $('adwordstopo').show(); } \
if ($('adwordstopo2')) { $('adwordstopo2').show(); } \
 \
} \
} \
 \
 \
if (ServerVariables.usuarioLogado == false && varPerfilN=='') { \
if ($('banners_internas')) {$('banners_internas').show(); } \
//if ($('bannerTeste')) {$('bannerTeste').hide(); } \
if ($('bannerNews')) {$('bannerNews').show(); } \
if ($('bannerAssine')) {$('bannerAssine').show(); \
if ($('adwords')) { $('adwords').hide(); } \
if ($('adwordstopo')) { $('adwordstopo').hide(); } \
if ($('adwordstopo2')) { $('adwordstopo2').hide(); } \
 \
} \
} \
 \
if (ServerVariables.menu == 'medicamentos') { \
if ($('adwordstopo')) { $('adwordstopo').hide(); } \
} \
if (ServerVariables.menu == 'medicamentos-injetaveis') { \
if ($('adwordstopo')) { $('adwordstopo').hide(); } \
} \
</script> \
 \
<!-- Mobile=False --> \
<!-- UrlDirect=home.htm --> \
<!-- Complete= --> \
       \
 \
					             \
							        \
							    \
							    \
							     \
							      <!-- Topo (Inicio) -->  \
								 <div id=\"topo\">       <!-- Topo Superior(Inicio) -->  \
								  <div id=\"topo_superior\">                \
								     <!-- Menu Superior (Inicio) -->   \
 \
<div id=\"menu_superior\"> \
    <ul> \
        <li><a href=\"http://www.medicinanet.com.br/sobre.htm\" class=\"menu_superior_esquerda\">Sobre</a></li> \
        <li><a href=\"http://www.medicinanet.com.br/editorial.htm\" class=\"menu_superior_meio\">Editorial</a></li> \
        <li><a href=\"http://www.medicinanet.com.br/corpo-editorial.htm\" class=\"menu_superior_meio\">Corpo  \
        Editorial</a></li> \
        <li><a href=\"http://www.medicinanet.com.br/faq.htm\" class=\"menu_superior_meio\">FAQ</a></li> \
        <li><a href=\"https://www.medicinanet.com.br/fale-conosco.htm?_mobile=off\" class=\"menu_superior_meio\">Fale Conosco</a></li>                     \
        <li><a href=\"https://www.medicinanet.com.br/assine.htm?_mobile=off\" class=\"menu_superior_direita\">Assinar</a></li> \
    </ul> \
</div> \
 \
 \
 \
 \
   \
									 <!-- Menu Superior (Fim) -->        <!-- Social Media Topo (Inicio) -->   \
									 <div id=\"social_media_topo\">      \
									  <a href=\"http://twitter.com/#!/medicinanet\" target=\"_blank\" class=\"twitter\"><img src=\"MedicinaNET_files/twitter_icon_topo.gif\" alt=\"Twitter\"></a>  \
									       <a href=\"http://www.facebook.com/MedicinaNET\" target=\"_blank\" class=\"facebook\"><img src=\"MedicinaNET_files/facebook_icon_topo.gif\" alt=\"Facebook\"></a>   \
										    <a href=\"http://www.medicinanet.com.br/medicinanet.rss\" class=\"feed\"><img src=\"MedicinaNET_files/feed_icon_topo.gif\" alt=\"Feed\"></a>  </div><!-- Social Media Topo (Fim) -->      <!-- Entrar Topo (Inicio) --> \
                            <div id=\"entrar_topo\"> \
                            <p>Já é assinante?</p> \
                            <a id=\"entrar-btn2\" name=\"entrar-bin2\" href=\"#\" onclick=\"if (document.getElementById('box-login').style.display == 'block') { document.getElementById('box-login').style.display = 'none'; } else { document.getElementById('box-login').style.display = 'block'; }\" class=\"entrar_btn\">Entrar</a> \
                            <div id=\"box-login\" name=\"box-login\"> \
                                <form name=\"login-form\" id=\"login-form\"> \
                                     <p> \
                                        <input id=\"input_user\" name=\"usuario\" class=\"login_campo\" value=\"E-Mail\" title=\"usuario\" tabindex=\"4\" type=\"text\" onblur=\"if(this.value=='') {this.value='E-Mail';}; window.activeForm = null;\" onfocus=\"if(this.value=='E-Mail') {this.value='';}; window.activeForm = 'loginTop';\"> \
                                    </p> \
                                     <p> \
                                        <input id=\"input_pwd\" name=\"senha\" class=\"login_campo\" value=\"Senha\" title=\"senha\" tabindex=\"5\" type=\"password\" onblur=\"if(this.value=='') {this.value='Senha';}; window.activeForm = null\" onfocus=\"if(this.value=='Senha') {this.value='';}; window.activeForm = 'loginTop';\"> \
                                     </p> \
                                     <p> \
                                        </p><p class=\"login_lembrar\"><input id=\"remember\" onclick=\"GravarAutoCookie();\" class=\"\" name=\"remember_me\" value=\"1\" tabindex=\"7\" type=\"checkbox\"> Lembrar</p> \
                                        <input id=\"login-submit\" class=\"login_submit\" onclick=\"execLoginBox()\" value=\"Entrar\" tabindex=\"6\" type=\"button\"> \
                                     <p></p> \
                                    <p><a href=\"http://www.medicinanet.com.br/senha.aspm\" class=\"esqueceu\">Esqueceu sua senha?</a></p>   \
                                </form> \
<div style=\"display:none\"><img src=\"MedicinaNET_files/loader2.gif\" border=\"0\"></div> \
                            </div> \
                            </div><!-- Entrar Topo (Fim) -->      </div><!-- Topo Superior (Fim) -->        <!-- Logo (Inicio) -->  <div class=\"logo\">  <a href=\"http://www.medicinanet.com.br/\"><img src=\"MedicinaNET_files/logo_medicinanet.gif\" class=\"logo\" alt=\"MedicinaNET\"></a>  </div><!-- Logo (Fim) -->      <!-- Busca (Inicio) --> \
                <div id=\"busca\"> \
                <input name=\"\" type=\"text\" id=\"busca_campo\" onkeypress=\"return EnterNaBusca(event)\" value=\"Busque doenças, remédios, artigos...\" onblur=\"if(this.value=='') {this.value='Busque doenças, remédios, artigos...';}\" onfocus=\"if(this.value=='Busque doenças, remédios, artigos...') {this.value='';}\"> \
                <input name=\"\" onclick=\"javascript:search();\" type=\"button\" value=\"Buscar\" id=\"busca_btn\"> \
                </div><!-- Busca (Fim) -->                     </div><!-- Topo (Fim) -->        <!-- Menu (Inicio) -->    <!-- Menu Start == --><!-- false,usuarioLogado: false --> \
<div id=\"menu\"> \
 \
    <ul> \
        <li><a href=\"http://www.medicinanet.com.br/\" class=\"inicialsel\"></a></li> \
         \
        <!-- <li><a href=\"/categorias/acp-medicine.htm?mobile=off\" class='acp'>Revisões<br /> \
      	Internacionais</a></li> \
--> \
         \
        <li><a href=\"http://www.medicinanet.com.br/categorias/revisoes.htm?mobile=off\" onmouseover=\"mostrarSubmenu('submenu-revisoes')\" onmouseout=\"ocultarSubmenu('submenu-revisoes')\" class=\"revisoes\"> \
		Revisões e <br> \
        Algoritmos</a> \
         \
 \
 \
 \
        <div id=\"submenu-revisoes\" class=\"submenu\" style=\"display: none;\"> \
         \
            <ul onmouseover=\"mostrarSubmenu('submenu-revisoes')\" onmouseout=\"ocultarSubmenu('submenu-revisoes')\"> \
                <li onclick=\"document.location.href = '/categorias/revisoes.htm?mobile=off'\"> \
				Revisões</li> \
                <li onclick=\"document.location.href = '/categorias/algoritmos.htm?mobile=off'\"> \
				Algoritmos</li> \
            </ul> \
         \
        </div> \
         \
        </li> \
         \
        <li><a href=\"http://www.medicinanet.com.br/categorias/aulas.htm?mobile=off\" class=\"aulas\">Aulas em <br> \
        Vídeo</a></li> \
         \
        <li><a href=\"http://www.medicinanet.com.br/categorias/artigos.htm?mobile=off\" class=\"artigos\">Artigos<br> \
		Comentados</a></li> \
         \
        <li><a href=\"http://www.medicinanet.com.br/categorias/casos.htm?mobile=off\" onmouseover=\"mostrarSubmenu('submenu-casos-clinicos')\" onmouseout=\"ocultarSubmenu('submenu-casos-clinicos')\" class=\"casos\"> \
		Casos <br> \
        Clínicos</a> \
         \
        <div id=\"submenu-casos-clinicos\" class=\"submenu\" style=\"display: none;\"> \
 \
            <ul onmouseover=\"mostrarSubmenu('submenu-casos-clinicos')\" onmouseout=\"ocultarSubmenu('submenu-casos-clinicos')\"> \
                <li onclick=\"document.location.href = '/categorias/casos.htm?ancor=2038'\"> \
				Prescrições</li> \
                <li onclick=\"document.location.href = '/categorias/casos.htm?ancor=1670'\"> \
				Casos Clínicos em Vídeo</li> \
                <li onclick=\"document.location.href = '/categorias/casos.htm?ancor=1624'\"> \
				Eletrocardiogramas</li> \
                <li onclick=\"document.location.href = '/categorias/casos.htm?ancor=1910'\"> \
				Imagens em Medicina</li> \
            </ul> \
         \
        </div> \
 \
 \
         \
        </li> \
         \
        <!-- <li><a href=\"/categorias/medcalc-3000.htm?mobile=off\" class='medcalc'>Calculadoras <br /> \
        Médicas</a></li> \
--> \
         \
        <li><a href=\"http://www.medicinanet.com.br/categorias/medicamentos.htm?mobile=off\" class=\"bpr\">BPR Guia de<br> \
		Remédios</a></li> \
         \
        <li><a href=\"http://www.medicinanet.com.br/categorias/biblioteca.htm?mobile=off\" onmouseover=\"mostrarSubmenu('submenu-biblioteca')\" onmouseout=\"ocultarSubmenu('submenu-biblioteca')\" class=\"biblioteca\"> \
		Biblioteca <br> \
        Livre</a> \
         \
        <div id=\"submenu-biblioteca\" class=\"submenu submenu_coluna\" onmouseover=\"mostrarSubmenu('submenu-biblioteca')\" onmouseout=\"ocultarSubmenu('submenu-biblioteca')\" style=\"display: none;\"> \
      \
            <ul> \
                <li onclick=\"document.location.href = '/categorias/medicamentos-injetaveis.htm?mobile=off'\"> \
				BPR Guia de Injetáveis<br><br>                                     </li> \
<li onclick=\"document.location.href = '/categorias/bulas_remedios.htm?mobile=off'\"> \
				Bulas de Medicamentos<br><br>                              </li>                 \
<li onclick=\"document.location.href = '/cid10.htm?mobile=off'\">CID 10</li> \
             \
               \
               <!-- <li onclick=\"document.location.href = 'http://wiki.medicinanet.com.br/conteudos/5/Biblioteca_Colaborativa_Aberta_Das_Instituicoes_de_Ensino_Medico.htm'\"/> \
				Wiki MedicinaNET</li> --> \
            </ul> \
            \
            <ul> \
                <!-- <li onclick=\"document.location.href = '/categorias/anti-infecciosos.htm'\"> \
				Guia de Anti-infecciosos da CCIH do HC-FMUSP</li> --> \
                <li onclick=\"document.location.href = '/categorias/biblioteca.htm?ancor=3120'\"> \
				Guias Livres do Ministério da Saúde</li> \
                <li onclick=\"document.location.href = '/categorias/biblioteca.htm?ancor=4155'\"> \
				Segurança do Paciente</li>                 \
                 \
            </ul> \
        </div> \
         \
        </li> \
         \
        <li><a href=\"http://www.medicinanet.com.br/categorias/qualidade-e-seguranca.htm\" class=\"outros\">Qualidade e segurança</a></li> \
         \
        \
    </ul> \
</div> \
<!-- SCRIPT PARA PUBLICO GERAL (inicio) --> \
<script language=\"javascript\" type=\"text/javascript\"> \
 \
//$('adwordstopo').hide(); \
var varPerfilN = ''; \
var varIdMarketing=''; \
 \
</script> \
<!-- SCRIPT PARA PUBLICO GERAL (fim) -->                     \
<script language=\"javascript\" type=\"text/javascript\"> \
</script> \
 \
  <!-- Menu (Fim) -->        <!-- Corpo (Inicio) -->  <div id=\"corpo\">  <!-- Google Adwords (Inicio) -->  <div id=\"adwordstopo\" style=\"display:none;margin: 10px auto;\" align=\"center\"><!-- Google Adwords (Inicio) --><!-- adwordstopo -->   </div><!-- Google Adwords (Fim) -->         <!-- Corpo Interno(Inicio) -->       <div id=\"corpo_interno\">                 <!-- conteudo-home (Inicio) --> \
       \
		          <div id=\"conteudo-home\" style=\"margin-bottom:30px;\"> \
                   \
		                                  <!-- conteudo-home-topo (Inicio) --> \
      \
		                                                  <div id=\"conteudo-home-topo\"> \
  \
		                                                                           <!-- home-banner (Inicio) --> \
                         <div id=\"home-banners\" style=\"overflow: hidden;\"><ul style=\"left: -1292px; width: 5168px;\"><li><div> \
<img src=\"MedicinaNET_files/20120328212623.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Artigos Comentados</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7819/beneficios_e_maleficios_da_terapia_anticoagulante_oral_na_doenca_renal_cronica.htm\"><font color=\"#FFFFFF\">Benefícios e Malefícios da Terapia Anticoagulante Oral na Doença Renal Crônica</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7819/beneficios_e_maleficios_da_terapia_anticoagulante_oral_na_doenca_renal_cronica.htm\"><font color=\"#FFFFFF\">Vamos avaliar os benefícios e os riscos comparados dos diferentes anticoagulantes orais em doentes renais crônicos.</font></a></p> \
                                </div></div></li><li><div> \
<img src=\"MedicinaNET_files/20120328212623.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Artigos Comentados</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7823/liraglutida_em_criancas_e_adolescentes_com_diabetes_tipo_2.htm\"><font color=\"#FFFFFF\">Liraglutida em Crianças e Adolescentes com Diabetes Tipo 2</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7823/liraglutida_em_criancas_e_adolescentes_com_diabetes_tipo_2.htm\"><font color=\"#FFFFFF\">Será que a liraglutida tem papel no tratamento do diabetes tipo 2 na área de Pediatria?</font></a></p> \
                                </div></div></li><li><div> \
<img src=\"MedicinaNET_files/20120328212847.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Revisões - Diagnóstico e Tratamento das Principais Doenças e Sintomas</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7820/sinusite_cronica.htm\"><font color=\"#FFFFFF\">Sinusite Crônica</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7820/sinusite_cronica.htm\"><font color=\"#FFFFFF\">Revisão sobre a definição, os tipos e o manejo da sinusite crônica.</font></a></p> \
                                </div></div></li><li><div> \
<img src=\"MedicinaNET_files/20120328212847.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Revisões - Diagnóstico e Tratamento das Principais Doenças e Sintomas</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7822/embolia_pulmonar.htm\"><font color=\"#FFFFFF\">Embolia Pulmonar</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7822/embolia_pulmonar.htm\"><font color=\"#FFFFFF\">Estudo e diagnóstico sobre a Embolia Pulmonar.</font></a></p> \
                                </div></div></li><li><div> \
<img src=\"MedicinaNET_files/20120328212847.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Revisões - Diagnóstico e Tratamento das Principais Doenças e Sintomas</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7815/eosinofilia_tropical.htm\"><font color=\"#FFFFFF\">Eosinofilia Tropical</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7815/eosinofilia_tropical.htm\"><font color=\"#FFFFFF\">Revisão sobre as manifestações clínicas e principalmente pulmonares da infecção pela Wuchereria.</font></a></p> \
                                </div></div></li><li><div> \
<img src=\"MedicinaNET_files/20120328212623.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Artigos Comentados</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7821/tratamentos_para_prevencao_e_manejo_do_suicidio.htm\"><font color=\"#FFFFFF\">Tratamentos para Prevenção e Manejo do Suicídio</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7821/tratamentos_para_prevencao_e_manejo_do_suicidio.htm\"><font color=\"#FFFFFF\">Vamos avaliar os benefícios e os malefícios de intervenções não farmacológicas e farmacológicas para prevenir o suicídio.</font></a></p> \
                                </div></div></li><li><div> \
<img src=\"MedicinaNET_files/20120328212623.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Artigos Comentados</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7816/risco_de_avc_em_pacientes_com_diabetes_tipo_2.htm\"><font color=\"#FFFFFF\">Risco de AVC em pacientes com Diabetes tipo 2</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/artigos/7816/risco_de_avc_em_pacientes_com_diabetes_tipo_2.htm\"><font color=\"#FFFFFF\">Existe um escore de risco melhor para avaliar a possibilidade de AVC em pacientes com diabetes melito tipo 2?</font></a></p> \
                                </div></div></li><li><div> \
<img src=\"MedicinaNET_files/20120328212847.png\" heigth=\"128\" class=\"home_banner_imagem\" style=\"margin-bottom:122px\" width=\"128\"> \
		<div class=\"home_banner_texto\"> \
                                    <p class=\"home_banner_categoria\">Revisões - Diagnóstico e Tratamento das Principais Doenças e Sintomas</p> \
                                    <h2><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7818/doenca_de_wilson.htm\"><font color=\"#FFFFFF\">Doença de Wilson</font></a></h2> \
                                    <p><a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/conteudos/revisoes/7818/doenca_de_wilson.htm\"><font color=\"#FFFFFF\">Revisão sobre o diagnóstico, os exames complementares e o manejo de pacientes com doença de Wilson (DW).</font></a></p> \
                                </div></div></li></ul></div> \
                        <!-- home-banner (Fim) --> \
                        <!-- home-banner-controle --> \
                        <div id=\"home-banner-controle\"> \
                        <div class=\"banner_controle_borda_esquerda\"> </div> \
                        <!-- home-banner-controle-botoes --> \
                        <div id=\"controle\"> \
                            <ul><li onclick=\"banner_home.pos(0)\" class=\"\">No 1</li><li onclick=\"banner_home.pos(1)\" class=\"\">No 2</li><li onclick=\"banner_home.pos(2)\" class=\"current\">No 3</li><li onclick=\"banner_home.pos(3)\" class=\"\">No 4</li><li onclick=\"banner_home.pos(4)\" class=\"\">No 5</li><li onclick=\"banner_home.pos(5)\" class=\"\">No 6</li><li onclick=\"banner_home.pos(6)\" class=\"\">No 7</li><li onclick=\"banner_home.pos(7)\" class=\"\">No 8</li></ul> \
                        </div><!-- home-banner-controle-botoes (fim)--> \
                         <div class=\"banner_controle_borda_direita\"> </div> \
                         </div> \
                         <!-- home-banner-controle (fim) --> \
					</div> \
                    <!-- conteudo-home-topo (Fim) -->  \
					<!-- conteudo-home-inferior (Inicio) --> \
                    <div id=\"conteudo-home-inferior\"> \
                                <!-- home_box (inicio) --> \
					            <div class=\"home_box\" style=\"height:222px\"> \
                                    <!-- home_box_conteudo (inicio) --> \
						            <div class=\"home_box_conteudo\"> \
							            <div class=\"home_box_texto\" style=\"height:159px\"><h2>Artigos Comentados</h2> \
                                         \
								        <p>29/07/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/artigos/7757/canaglifozina_e_desfechos_renais_em_nefropatia_por_diabetes_tipo_2.htm\">Canaglifozina e Desfechos Renais em Nefropatia por Diabetes Tipo 2</a></strong> - Lucas Santos Zambon</p><p> \
								        </p><p>&nbsp;</p> \
                                         \
								        <p>29/07/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/artigos/7755/compressao_pneumatica_intermitente_como_adjuvante_em_profilaxia_de_tev.htm\">Compressão Pneumática Intermitente como adjuvante em profilaxia de TEV</a></strong> - Lucas Santos Zambon</p><p> \
								        </p></div> \
                                    <div class=\"home_box_vertodos_btn\"> \
								        <a href=\"http://www.medicinanet.com.br/categorias/artigos.htm\">Ver todos</a> \
							        </div> \
						        </div> \
						        <!-- home_box_conteudo (fim) --> \
					        </div> \
					        <!-- home_box (fim) --> \
                                <!-- home_box (inicio) --> \
					            <div class=\"home_box\" style=\"height:222px\"> \
                                    <!-- home_box_conteudo (inicio) --> \
						            <div class=\"home_box_conteudo\"> \
							            <div class=\"home_box_texto\" style=\"height:159px\"><h2>Aulas em Vídeo</h2> \
                                        <div class=\"home_box_imagem\"><a href=\"http://www.medicinanet.com.br/videos/aulas/663/estado_de_mal_asmatico_no_pronto_socorro_pediatrico.htm\"><img src=\"MedicinaNET_files/bg_home_video_miniatura.png\"></a></div> \
								        <p>07/12/2017</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/videos/aulas/663/estado_de_mal_asmatico_no_pronto_socorro_pediatrico.htm\">Estado de Mal Asmatico no Pronto Socorro Pediatrico</a></strong> - </p><p> \
								        </p><p>&nbsp;</p></div> \
                                    <div class=\"home_box_vertodos_btn\"> \
								        <a href=\"http://www.medicinanet.com.br/categorias/aulas.htm\">Ver todos</a> \
							        </div> \
						        </div> \
						        <!-- home_box_conteudo (fim) --> \
					        </div> \
					        <!-- home_box (fim) --> \
                                <!-- home_box (inicio) --> \
					            <div class=\"home_box\" style=\"height:222px\"> \
                                    <!-- home_box_conteudo (inicio) --> \
						            <div class=\"home_box_conteudo\"> \
							            <div class=\"home_box_texto\" style=\"height:159px\"><h2>Revisões Internacionais</h2> \
                                         \
								        <p>17/06/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/acp-medicine/7739/neurologia_feminina.htm\">Neurologia Feminina</a></strong> - </p><p> \
								        </p><p>&nbsp;</p> \
                                         \
								        <p>26/04/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/acp-medicine/7720/parkinsonismo_e_disturbios_relacionados.htm\">Parkinsonismo e Distúrbios Relacionados</a></strong> - </p><p> \
								        </p></div> \
                                    <div class=\"home_box_vertodos_btn\"> \
								        <a href=\"http://www.medicinanet.com.br/categorias/acp-medicine.htm\">Ver todos</a> \
							        </div> \
						        </div> \
						        <!-- home_box_conteudo (fim) --> \
					        </div> \
					        <!-- home_box (fim) --> \
                                <!-- home_box (inicio) --> \
					            <div class=\"home_box\" style=\"height:222px\"> \
                                    <!-- home_box_conteudo (inicio) --> \
						            <div class=\"home_box_conteudo\"> \
							            <div class=\"home_box_texto\" style=\"height:159px\"><h2>Revisões e Algoritmos</h2> \
                                         \
								        <p>08/07/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/7754/enxaqueca.htm\">Enxaqueca</a></strong> - Rodrigo Antonio Brandão Neto</p><p> \
								        </p><p>&nbsp;</p> \
                                         \
								        <p>08/07/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/7752/hipemagnesemia.htm\">Hipemagnesemia</a></strong> - Rodrigo Antonio Brandão Neto</p><p> \
								        </p></div> \
                                    <div class=\"home_box_vertodos_btn\"> \
								        <a href=\"http://www.medicinanet.com.br/categorias/revisoes.htm\">Ver todos</a> \
							        </div> \
						        </div> \
						        <!-- home_box_conteudo (fim) --> \
					        </div> \
					        <!-- home_box (fim) --> \
                                <!-- home_box (inicio) --> \
					            <div class=\"home_box\" style=\"height:222px\"> \
                                    <!-- home_box_conteudo (inicio) --> \
						            <div class=\"home_box_conteudo\"> \
							            <div class=\"home_box_texto\" style=\"height:159px\"><h2>Casos Clínicos</h2> \
                                         \
								        <p>12/06/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/casos/7737/paciente_de_44_anos_de_idade_do_sexo_masculino_com_antecedente_de_hanseniase.htm\">Paciente de 44 Anos de Idade do Sexo Masculino, com Antecedente de Hanseníase</a></strong> - Rodrigo Antonio Brandão Neto</p><p> \
								        </p><p>&nbsp;</p> \
                                         \
								        <p>03/06/2019</p> \
								        <p><strong><a href=\"http://www.medicinanet.com.br/conteudos/casos/7733/paciente_de_48_anos_de_idade_sexo_feminino_deu_entrada_no_departamento_de_emergencia_com_quadro_de_p.htm\">Paciente de 48 Anos de Idade Sexo Feminino deu Entrada no Departamento de Emergência com Quadro de P</a></strong> - Rodrigo Antonio Brandão Neto</p><p> \
								        </p></div> \
                                    <div class=\"home_box_vertodos_btn\"> \
								        <a href=\"http://www.medicinanet.com.br/categorias/casos.htm\">Ver todos</a> \
							        </div> \
						        </div> \
						        <!-- home_box_conteudo (fim) --> \
					        </div> \
					        <!-- home_box (fim) --> \
                	<div class=\"home_box\" style=\"height:222px\"> \
						<!-- home_box_conteudo (inicio) --> \
						<div class=\"home_box_conteudo\"> \
							<div class=\"home_box_texto\" id=\"editorialhome\" style=\"height:159px\"> \
<h2>Editorial</h2> \
<h5><b>Editorial MedicinaNET - Dezembro - 2018</b></h5> \
 \
								 \
<p>Custos em Saúde e Cadeia de Suprimentos \
  \
  \
A grande parte dos custos nas instituições prestadoras de serviços à  \
saúde e, consequentemente, no sistema de saúde, se deve aos mate (...)</p> \
		 \
</div> \
							<div class=\"home_box_leiamais_btn\"> \
								<a href=\"http://www.medicinanet.com.br/editorial.htm\">Leia mais</a> \
							</div> \
						</div> \
						<!-- home_box_conteudo (fim) --> \
					</div> \
 \
      \
		                                                                                                   <script type=\"text/javascript\"> \
        \
		                                                                                                                           var banner_home = new TINY.slider.slide(\"banner_home\",{                                    id:\"home-banners\",                                    auto: 4,                                    resume:false,                                    vertical:false,                                    navid:\"controle\",                                    activeclass:\"current\",                                    position:0,                                    rewind:false,                                    elastic:false                               });                          </script> \
                                                                            <div class=\"clear\"> \
</div> \
                     </div> \
                     <!-- conteudo-home-inferior (Fim) --> \
                                                                               </div> \
                <!-- conteudo-home (Fim) --> \
				 \
				 \
				 \
                                <!-- conteudo-home-sidebar (Inicio) --> \
                <div id=\"conteudo-home-sidebar\"> \
                     <!-- Benners Home --> \
                     <div id=\"banners_home\"> \
                           \
		                                                                                                                             \
                  <!--  <a href=\"/teste-gratis.htm\" class=\"banner_teste\">Conheça o maior  portal médico do  Brasil. Faça um teste gratuito de 5 dias.</a>   -->                   \
                    <a href=\"https://www.medicinanet.com.br/assine.htm\" class=\"banner_assine\">Assine</a>   \
 \
 \
<!-- SCRIPT PARA PUBLICO GERAL (inicio) --> \
<script language=\"javascript\" type=\"text/javascript\"> \
$('adwordstopo').hide(); \
//var varPerfilN = ''; \
 \
 \
</script> \
<!-- SCRIPT PARA PUBLICO GERAL (fim) --> \
                 \
    \
		                                                                                                                            <!-- sidebar-banners-ts --> \
   \
		                                                                                                          <div class=\"sidebar_box_bannerTS\"> \
        \
		                                                                       \
                                                                         \
             <!-- sidebar-banners-conteudo --> \
                            <div id=\"sidebar-banners-ts\"> \
                                 <!-- sidebar-banner-ts (Inicio) --> \
                                 <div style=\"overflow: hidden;\" id=\"sidebar-banner-ts\" name=\"sidebar-banner-ts\"> \
                                <ul style=\"left: -457px; width: 687px;\"> \
                                                                 <li> \
<div> \
<h2>Temas Selecionados</h2> \
<p><strong> \
<a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/categorias/temas-selecionados.htm?ancor=11999\">Artigos em Destaque</a> \
</strong></p> \
</div> \
</li><li> \
<div> \
<h2>Temas Selecionados</h2> \
<p><strong> \
<a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/categorias/temas-selecionados.htm?ancor=12054\">Revisões em Destaque</a> \
</strong></p> \
</div> \
</li><li> \
<div> \
<h2>Temas Selecionados</h2> \
<p><strong> \
<a style=\"text-decoration: none\" href=\"http://www.medicinanet.com.br/categorias/temas-selecionados.htm?ancor=12092\">Casos Clínicos em Destaque</a> \
</strong></p> \
</div> \
</li> \
                                                                 </ul> \
                                 </div> \
                                 <!-- sidebar-banner-ts (Fim) --> \
                                 <!-- Seta esquerda navegação Banner (inicio)                                  <div class=\"controle_banner_seta_esquerda\"> \
                                      <a class=\"prev\"> \
 <- </a> \
                                 </div> \
                                 <!-- Seta esquerda navegação Banner (fim) --> \
                                                                                                 <!-- Seta direita navegação Banner (inicio)                                  <div class=\"controle_banner_seta_direita\"> \
                                      <a class=\"next\"> \
 </a> \
                                 </div> \
                               <!-- Seta direita navegação Banner (fim) --> \
                                   <!-- sidebar-banner-controle --> \
                                 <center> \
                               <div id=\"sidebar-ts-banner-controle\" name=\"sidebar-ts-banner-controle\"> \
                                                                          <!-- borda bg controle esquerda (inicio) --> \
                                                             <!-- SCRIPT PARA HOME-BANNERS --> \
                                \
		                                                                       \
                                                                         \
             <script> \
                               var sidebar_banner_ts = new TINY.slider.slide(\"sidebar_banner_ts\",{                                   id:\"sidebar-banner-ts\",                                   auto: 5,                                   resume:true,                                   vertical:false,                                                                     position:0,                                   rewind:true,                                   elastic:false                               });                             </script> \
                                <!-- SCRIPT PARA HOME-BANNERS (fim) --> \
                                                                     \
		                                                                       \
                                                                         \
             <!-- sidebar-banner-controle-botoes (fim)--> \
                                        <!-- borda bg controle direita (inicio) --> \
                                                                         <!-- borda bg controle direita (inicio) --> \
                                                                                                      </div> \
                                 </center> \
                                 <!-- sidebar-banner-controle (fim) --> \
                                                                                          </div> \
                                                  <!-- conteudo-home-sidebar (Fim) --> \
  \
		                                                                       \
                                                                         \
             </div> \
                                                          <a href=\"http://www.medicinanet.com.br/newsletter.aspm\" class=\"newsletter_btn\"> \
Newsletter</a> \
                          <a href=\"http://medicinanet2.ourtoolbar.com/\" target=\"_blank\" class=\"toolbar_btn\"> \
Toolbar</a> \
                          <a href=\"http://www.medicinanet.com.br/fale-conosco.htm\" class=\"fale_conosco_btn\"> \
Fale Conosco</a> \
                     </div> \
                     <!-- Banners Home (fim) --> \
              \
        <!-- sidebar-banners-topo --> \
<!-- \
                     <div class=\"sidebar_box_banner\"> \
                                                    \
                          <div id=\"sidebar-banners-topo\"> \
                                \
                               <div id=\"sidebar-banner-topo\" name=\"sidebar-banner-topo\"> \
                              <ul> \
                                  <li> \
                                      <div class=\"banner_arredondado\"> \
   \
                                        <a target=\"_blank\" href=\"http://www.grupoa.com.br/livros/clinica-medica-e-semiologia/clinica-medica/9788565852173\"> \
<img src=\"/img/banner_mednet.gif\" /> \
</a> \
                                    </div> \
                                 </li> \
                                                              </ul> \
                               </div> \
                                                            <div class=\"controle_banner_seta_esquerda\"> \
                                    <a class=\"prev\"> \
 <- </a> \
                               </div> \
                              \
                                                                                             <!-- Seta direita navegação Banner (inicio)                                <div class=\"controle_banner_seta_direita\"> \
                                    <a class=\"next\"> \
 </a> \
                               </div> \
                              \
                               <center> \
                               <div id=\"sidebar-topo-banner-controle\" name=\"sidebar-topo-banner-controle\"> \
                                                                       \
                                    <div class=\"sidebar_banner_controle_borda_esquerda\"> \
 </div> \
                                     \
                                    \
                                    <div id=\"sidebar-topo-controle\" name=\"sidebar-topo-controle\"> \
                                                                                  <ul> \
                                     <li onclick=\"sidebar_banner_topo.pos(0)\"> \
No 1</li> \
                                 </ul> \
                                                                             </div> \
                                                           \
                             <script> \
                             var sidebar_banner_topo = new TINY.slider.slide(\"sidebar_banner_topo\",{                                 id:\"sidebar-banner-topo\",                                 auto: 5,                                 resume:true,                                 vertical:false,                                 navid:\"sidebar-topo-controle\",                                 activeclass:\"current\",                                 position:0,                                 rewind:true,                                 elastic:false                             });                             </script> \
                               \
                                                                  \
                                      \
                                    <div class=\"sidebar_banner_controle_borda_direita\"> \
 </div> \
                                   \
                                                                                                  </div> \
                               </center> \
                               \
                                                                                        </div> \
                          \
                                                                              <div id=\"controle-sidebar\" name=\"controle-sidebar\"> \
                                                         </div> \
                                               </div> \
                      \
                                                               \
                     <div class=\"sidebar_box_banner\"> \
                                                     \
                          <div id=\"sidebar-banners-baixo\" name=\"sidebar-banners-baixo\"> \
                                                                                              \
                               <div id=\"sidebar-banner-baixo\" name=\"sidebar-banner-baixo\"> \
                                   <ul> \
                                       <li> \
                                          <div> \
                                             <a target=\"_blank\" href=\"http://www.semcad.com.br\"> \
<img src=\"/img/ArtmedPanamericana.gif\" /> \
</a> \
                                          </div> \
                                      </li> \
                                    </ul> \
                                                               \
                                \
                               <center> \
                               <div id=\"sidebar-baixo-banner-controle\" name=\"sidebar-baixo-banner-controle\"> \
                                                                         \
                                    <div class=\"sidebar_banner_controle_borda_esquerda\"> \
 </div> \
                                   \
                                       \
                                    <div id=\"sidebar-baixo-controle\" name=\"sidebar-baixo-controle\"> \
                                                                                  <ul> \
                                     <li onclick=\"sidebar_banner_baixo.pos(0)\"> \
No 1</li> \
                                   </ul> \
                                                                             </div> \
                                            -->              <!-- SCRIPT PARA HOME-BANNERS --> \
                             <script> \
                             var sidebar_banner_baixo = new TINY.slider.slide(\"sidebar_banner_baixo\",{                                 id:\"sidebar-banner-baixo\",                                 auto: 5,                                 resume:true,                                 vertical:false,                                 navid:\"sidebar-baixo-controle\",                                 activeclass:\"current\",                                 position:0,                                 rewind:false,                                 elastic:false                             });                             </script> \
                             <!-- SCRIPT PARA HOME-BANNERS (fim) --> \
  <!-- SCRIPT PARA PUBLICO GERAL (inicio) --> \
 <script language=\"javascript\" type=\"text/javascript\"> \
 if (varPerfilN==\"publico\") { if (ServerVariables.pageName == \"home\") { if (ServerVariables.usuarioLogado) {  } else {     AbrirPerfilPublico();  } } } </script> \
 <!-- SCRIPT PARA PUBLICO GERAL (fim) --> \
                                                                  <!-- sidebar-banner-controle-botoes (fim)--> \
<!-- borda bg controle direita (inicio) --> \
<!-- \
                                    <div class=\"sidebar_banner_controle_borda_direita\"> \
 </div> \
 --> \
                                    <!-- borda bg controle direita (inicio) --> \
                                                                                                  </div> \
                                \
                               <!-- sidebar-banner-controle (fim) --> \
                                                         </div> \
                          <!-- sidebar-banners-conteudo --> \
                                               </div> \
                     <!-- sidebar-banners-topo (fim) --> \
					  <div style=\"margin-top:30px;\"> <!-- --> <br> <p></p></div>  \
					 <div id=\"adwordstopo2\" style=\"display:none;margin: 20px auto;\" align=\"center\"> <br> \
		                                             <!-- adwordstopo -->                                                                                                           \
													  \
													 </div>     \
                                      \
                \
		                                                                       \
                  \
		                                                                       \
                                                                         \
              \
		                                                                       \
                                                                         \
           <div class=\"class=\" sidebar_box_banner\"\"=\"\"> \
      \
		                                                                       \
                                                                         \
                </div> \
                                     <!-- Corpo Interno (Fim) -->         <div class=\"clear\"></div>  \
											 <!-- Rodape-Bulas (inicio) --><!-- adwordstopo -->    <!-- Rodape-Bulas (fim) --> \
											            <!-- Corpo (Fim) -->        <div class=\"clear\"></div>    <!-- Rodape 1 (Inicio) -->  <div id=\"rodape1\">    <!-- Rodape 1 Interno (Inicio) -->  <div id=\"rodape1_interno\">    <!-- Mais Acessados (Inicio) -->  <div id=\"mais_acessados\">  <h2>Conteúdos Mais Acessados</h2>  <ul><li><a href=\"http://www.medicinanet.com.br/categorias/bulas_remedios.htm\">Bulas</a></li><li><a href=\"http://www.medicinanet.com.br/categorias/revisoes.htm\">Revisões</a></li><li><a href=\"http://www.medicinanet.com.br/categorias/cid10.htm\">CID 10</a></li></ul><ul><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1615/dengue.htm\">Dengue</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1526/tuberculose.htm\">Tuberculose</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1270/toxoplasmose.htm\">Toxoplasmose</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1287/ginecomastia.htm\">Ginecomastia</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1543/hipotireoidismo.htm\">Hipotireoidismo</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1524/leptospirose.htm\">Leptospirose</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1253/tontura_e_vertigem.htm\">Vertigem e Tontura</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/48/ataque_isquemico_transitorio_e_acidente_vascular_cerebral.htm\">Acidente Vascular Cerebral</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1389/rouquidao.htm\">Rouquidão</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1329/perda_de_peso.htm\">Perda de Peso</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1372/hematuria.htm\">Hematúria</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1238/febre_reumatica.htm\">Febre Reumática</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1399/halitose.htm\">Halitose</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/160/amoxicilina.htm\">Amoxicilina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/491/fluoxetina.htm\">Fluoxetina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/479/fluconazol.htm\">Fluconazol</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/508/furosemida.htm\">Furosemida</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/938/sertralina.htm\">Sertralina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/259/diazepan.htm\">Diazepan</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/289/dopamina.htm\">Dopamina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/1119/prednisona.htm\">Prednisona</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/942/sinvastatina.htm\">Sinvastatina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/919/risperidona.htm\">Risperidona</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/994/tetraciclina_oral_.htm\">Tetraciclina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/695/venlafaxina.htm\">Venlafaxina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/595/clopidogrel.htm\">Clopidogrel</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/178/atorvastatina.htm\">Atorvastatina</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/287/domperidona.htm\">Domperidona</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/791/metronidazol_oral_injetavel_.htm\">Metronidazol</a></li><li><a href=\"http://www.medicinanet.com.br/conteudos/medicamentos/554/cetoconazol_oral_.htm\">Cetoconazol</a></li></ul>  </div><!-- Mais Acessados (Fim) -->    <!-- Twitter (Inicio) -->  <div id=\"twitter\"><!-- <iframe id=\"TwitterList\" name=\"TwitterList\" class=\"TwitterList\" frameborder=\"0\" marginheight=\"0\" marginwidth=\"0\" scrolling=\"no\" src=\"/twitter.html\" width=\"266\" height=\"256\" allowtransparency=\"true\"></iframe>  --> </div><!-- Twitter (Fim) -->    </div><!-- Rodape 1 Interno (Fim) -->  </div><!-- Rodape 1 (Fim) -->    <!-- Rodape 2 (Inicio) -->  <div id=\"rodape2\">  <div id=\"rodape2_interno\">    <!-- Rodape 2 Superior (Inicio) -->  <div id=\"rodape2_superior\">    <!-- Social Media Rodape (Inicio) -->  <div id=\"social_media_rodape\">       <h2>Conecte-se</h2>      <a href=\"http://twitter.com/#!/medicinanet\" class=\"twitter\"><img src=\"MedicinaNET_files/twitter_icon_rodape.gif\" alt=\"Twitter\"></a>      <a href=\"http://www.facebook.com/MedicinaNET\" class=\"facebook\"><img src=\"MedicinaNET_files/facebook_icon_rodape.gif\" alt=\"Facebook\"></a>  <a href=\"https://plus.google.com/u/0/111387230036070189545/\" target=\"_blank\" class=\"facebook\"><img src=\"MedicinaNET_files/googleplus_icon_rodape.gif\" alt=\"Google+\"></a>     <a href=\"http://www.medicinanet.com.br/medicinanet.rss\" class=\"feed\"><img src=\"MedicinaNET_files/feed_icon_rodape.gif\" alt=\"Feed\"></a>  </div><!-- Social Media rodape (Fim) -->      <!-- Sobre o MedicinaNET (Inicio) -->  <div id=\"sobre_o_medicinanet\">       <h2>Sobre o MedicinaNET</h2>      <p align=\"justify\">O \
 MedicinaNET é o maior portal médico em português. Reúne recursos  \
indispensáveis e conteúdos de ponta contextualizados à realidade  \
brasileira, sendo a melhor ferramenta de consulta para tomada de  \
decisões rápidas e eficazes.</p>  </div><!-- Sobre o MedicinaNET (Fim) -->    <!-- Logo Grupoa (Inicio) -->  <div id=\"logo_grupoa\">       <a href=\"http://www.grupoa.com.br/\" target=\"_blank\"><img src=\"MedicinaNET_files/logo_grupoa.gif\"></a>      <a href=\"http://www.secad.com.br/\" target=\"_blank\"><img src=\"MedicinaNET_files/logo_artmed.gif\"></a>  </div><!-- Logo Grupoa (Fim) -->    </div><!-- Rodape 2 Superior (Fim) -->    <!-- Rodape 2 Inferior (Inicio) --><div style=\"font-family: Arial,Helvetica,sans-serif;font-color:#666;font-size:11px;height:50px;margin-top:10;padding-top:4px\"><table width=\"100%\" cellspacing=\"3\" cellpadding=\"3\" border=\"0\"><tbody><tr><td width=\"506\" valign=\"top\" bgcolor=\"#FFFFFF\">Medicinanet Informações de Medicina S/A<br>Av. Jerônimo de Ornelas, 670, Sala  501<br>Porto Alegre, RS 90.040-340<br>Cnpj: 11.012.848/0001-57</td> <td width=\"77a\" valign=\"top\" bgcolor=\"#FFFFFF\">(51) 3093-3131<br>info@medicinanet.com.br</td></tr></tbody></table><br><br></div><div id=\"rodape2_inferior\">      <p class=\"esquerda\">MedicinaNET - Todos os direitos reservados.</p>      <p class=\"direita\"><!-- <a href=\"/politica-privacidade.htm\">Política de       Privacidade</a>   |   --><a href=\"http://www.medicinanet.com.br/termo-uso.htm\">Termos de Uso do Portal</a></p>  </div>    </div><!-- Rodape 2 Inferior (Fim) -->  </div><!-- Rodape 2 (Fim) -->          <!-- BEGIN: Google Analytics -->    <script type=\"text/javascript\" src=\"MedicinaNET_files/ga.js\"></script> \
                            <script type=\"text/javascript\">  var pageTracker = _gat._getTracker(\"UA-5780314-1\");  \
pageTracker._setCustomVar(1,\"Logged\",\"N\",3);  \
 \
pageTracker._trackPageview(); \
</script>     <!-- END: Google Analytics -->        \
														  \
											 <script language=\"javascript\" type=\"text/javascript\">   \
											  \
											   if (ServerVariables.usuarioLogado) {  } else {  if (varPerfilN==\"medico\") {      if ($(\"banners_internas\")) {$(\"banners_internas\").show(); }    if ($(\"bannerNews\")) {$(\"bannerNews\").show(); }  if ($(\"bannerAssine\")) {$(\"bannerAssine\").show(); }  if ($(\"box-twitter\")) { $(\"box-twitter\").show(); }  if ($(\"facebook-box\")) { $(\"facebook-box\").show(); }  if ($(\"adwords\")) { $(\"adwords\").hide(); }  if ($(\"adwordstopo\")) { $(\"adwordstopo\").hide(); $(\"adwordstopo2\").hide();}  }    if (varPerfilN==\"publico\") {      if ($(\"banners_internas\")) {$(\"banners_internas\").show(); }  if ($(\"bannerNews\")) {$(\"bannerNews\").show(); }  if ($(\"bannerAssine\")) {$(\"bannerAssine\").show(); }  if ($(\"box-twitter\")) { $(\"box-twitter\").show(); }  if ($(\"facebook-box\")) { $(\"facebook-box\").show(); }  if ($(\"adwords\")) { $(\"adwords\").show(); }  if ($(\"adwordstopo\")) { $(\"adwordstopo\").show();$(\"adwordstopo2\").show(); }  }  if (varPerfilN==\"profissional\") {      if ($(\"banners_internas\")) {$(\"banners_internas\").show(); }   if ($(\"bannerNews\")) {$(\"bannerNews\").show(); }  if ($(\"bannerAssine\")) {$(\"bannerAssine\").show(); }  if ($(\"box-twitter\")) { $(\"box-twitter\").show(); }  if ($(\"facebook-box\")) { $(\"facebook-box\").show(); }  if ($(\"adwords\")) { $(\"adwords\").show(); }  if ($(\"adwordstopo\")) { $(\"adwordstopo\").show();$(\"adwordstopo2\").show(); }  }  if (varPerfilN==\"estudante\") {      if ($(\"banners_internas\")) {$(\"banners_internas\").show(); }     if ($(\"bannerNews\")) {$(\"bannerNews\").show(); }  if ($(\"bannerAssine\")) {$(\"bannerAssine\").show(); }  if ($(\"adwords\")) { $(\"adwords\").show(); }  if ($(\"adwordstopo\")) { $(\"adwordstopo\").show();$(\"adwordstopo2\").show(); }  }  }      if (ServerVariables.usuarioLogado == false && varPerfilN==\"\") {  if ($(\"banners_internas\")) {$(\"banners_internas\").show(); }   if ($(\"bannerNews\")) {$(\"bannerNews\").show(); }  if ($(\"bannerAssine\")) {$(\"bannerAssine\").show();  if ($(\"adwords\")) { $(\"adwords\").hide(); }  if ($(\"adwordstopo\")) { $(\"adwordstopo\").hide();$(\"adwordstopo2\").hide(); }  }  }    if (ServerVariables.menu == \"medicamentos\") {  if ($(\"adwordstopo\")) { $(\"adwordstopo\").hide();$(\"adwordstopo2\").hide(); }  }  if (ServerVariables.menu == \"medicamentos-injetaveis\") {  if ($(\"adwordstopo\")) { $(\"adwordstopo\").hide();$(\"adwordstopo2\").hide(); }  }     \
											    \
											    \
						 \
 \
								 \
 \
											             </script>  \
														 <script language=\"javascript\" type=\"text/javascript\"> \
														  \
														    \
    var is_safari = navigator.userAgent.indexOf(\"Safari\") > -1; \
     \
    if ((is_safari)) {} else { \
     \
														 window.setTimeout(\"window.onresize();\",2000); \
														 } \
														  \
														  \
														  \
														  \
														 if (ServerVariables.usuarioLogado) {  if ($(\"adwordstopo\")) { $(\"adwordstopo\").hide(); } \
if ($(\"adwordstopo2\")) {$(\"adwordstopo2\").hide(); } \
if ($(\"banners_internas\")) {$(\"banners_internas\").hide(); } }  \
 \
														 </script> \
											    \
									 \
											   <script type=\"text/javascript\" class=\"teads\" async=\"true\" src=\"MedicinaNET_files/tag.js\"></script> \
											             \
	</body></html>", ""},
    {"lhb", 0, 1, "<!DOCTYPE html SYSTEM \"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/content/dtd/lhb_custom.dtd\"> \
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\"><head> \
<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"><script src=\"Laboratory%20Hazards%20Bulletin_files/cbgapi.loaded_0\" async=\"\"></script> \
    <meta charset=\"utf-8\"> \
    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> \
    <!-- RSC FAVICONS --> \
    <link rel=\"shortcut icon\" href=\"https://www.rsc-cdn.org/oxygen/assets/favicons/favicon.ico\" type=\"image/x-icon\"> \
    <link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"https://www.rsc-cdn.org/oxygen/assets/favicons/apple-touch-icon.png\"> \
    <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"https://www.rsc-cdn.org/oxygen/assets/favicons/favicon-32x32.png\"> \
    <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"https://www.rsc-cdn.org/oxygen/assets/favicons/favicon-16x16.png\"> \
    <link rel=\"manifest\" href=\"https://www.rsc-cdn.org/oxygen/assets/favicons/site.webmanifest\"> \
    <link rel=\"mask-icon\" href=\"https://www.rsc-cdn.org/oxygen/assets/favicons/safari-pinned-tab.svg\" color=\"#5bbad5\"> \
    <meta name=\"msapplication-TileColor\" content=\"#2d89ef\"> \
    <meta name=\"theme-color\" content=\"#ffffff\"> \
 \
            <!-- Google Tag Manager --> \
            <script type=\"text/javascript\" async=\"\" src=\"Laboratory%20Hazards%20Bulletin_files/ga.js\"></script><script async=\"\" src=\"Laboratory%20Hazards%20Bulletin_files/gtm.js\"></script><script>(function (w, d, s, l, i) { w[l] = w[l] || []; w[l].push({ 'gtm.start': new Date().getTime(), event: 'gtm.js' }); var f = d.getElementsByTagName(s)[0], j = d.createElement(s), dl = l != 'dataLayer' ? '&l=' + l : ''; j.async = true; j.src = 'https://www.googletagmanager.com/gtm.js?id=' + i + dl; f.parentNode.insertBefore(j, f); })(window, document, 'script', 'dataLayer', 'GTM-KXLLNJR');</script> \
            <!-- End Google Tag Manager --> \
 \
 \
    <!-- END: RSC FAVICONS --> \
            <link href=\"Laboratory%20Hazards%20Bulletin_files/FeedBack.css\" rel=\"stylesheet\" type=\"text/css\"> \
            <link href=\"Laboratory%20Hazards%20Bulletin_files/bootstrap-non-responsive.css\" rel=\"stylesheet\" type=\"text/css\"> \
            <link href=\"Laboratory%20Hazards%20Bulletin_files/main-non-responsive.css\" rel=\"stylesheet\" type=\"text/css\"> \
 \
 \
    <link href=\"Laboratory%20Hazards%20Bulletin_files/ScrollToTop.css\" rel=\"stylesheet\"> \
    <link href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/favicon.ico\" rel=\"shortcut icon\" type=\"image/x-icon\"> \
    <link href=\"Laboratory%20Hazards%20Bulletin_files/museo-sans-500_002.css\" rel=\"stylesheet\" type=\"text/css\"> \
    <link href=\"Laboratory%20Hazards%20Bulletin_files/carousel.css\" rel=\"stylesheet\" type=\"text/css\"> \
    <link href=\"Laboratory%20Hazards%20Bulletin_files/thickbox.css\" rel=\"stylesheet\" type=\"text/css\"> \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/jquery-1.js\" type=\"text/javascript\"></script><style>@media print {#ghostery-purple-box {display:none !important}}</style> \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/jquery_004.js\" type=\"text/javascript\"></script> \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/jquery_002.js\" type=\"text/javascript\"></script> \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/bootstrap.js\" type=\"text/javascript\"></script> \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/jquery_003.js\" type=\"text/javascript\"></script> \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/carousel_script.js\" type=\"text/javascript\"></script> \
 \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/thickbox.js\" type=\"text/javascript\"></script> \
 \
    <!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries --> \
    <!--[if lt IE 9]> \
       <script src=\"/lus/laboratory-hazards-bulletin/Scripts/html5shiv.min.js?1.0.0.12\" type=\"text/javascript\"></script> \
 \
       <script src=\"/lus/laboratory-hazards-bulletin/Scripts/respond.min.js\" type=\"text/javascript\"></script> \
 \
       <link href=\"/lus/laboratory-hazards-bulletin/Content/HtmlFiles/respond-proxy.html\" id=\"respond-proxy\" rel=\"respond-proxy\" /> \
       <link href=\"/lus/laboratory-hazards-bulletin/Content/Images/respond.proxy.gif\" id=\"respond-redirect\" rel=\"respond-redirect\" /> \
       <script src=\"/lus/laboratory-hazards-bulletin/Scripts/respond.proxy.js\" type=\"text/javascript\"></script> \
 \
     <style> \
           .social_bar{ \
    margin-left:1000px !important; \
    } \
    .breadcrumb li{ \
     display:block; \
     float:left; \
    } \
    </style> \
    <![endif]--> \
    <script type=\"text/javascript\"> \
        $(window).load(function () { \
            $.getScript(\"http://s7.addthis.com/js/250/addthis_widget.js#pubid=xa-4e365bb73ff2de63\"); \
            $.getScript(\"https://apis.google.com/js/plusone.js\"); \
        }); \
        $(document).ready(function () { \
            LoadBranding(); \
 \
            //<![CDATA[ \
            (function (e, d, b) { var a = 0; var f = null; var c = { x: 0, y: 0 }; e(\"[data-toggle]\").closest(\"li\").on(\"mouseenter\", function () { if (f) { f.removeClass(\"open\") } d.clearTimeout(a); f = e(this); a = d.setTimeout(function () { f.addClass(\"open\") }, b) }).on(\"mousemove\", function (g) { if (Math.abs(c.x - g.ScreenX) > 4 || Math.abs(c.y - g.ScreenY) > 4) { c.x = g.ScreenX; c.y = g.ScreenY; return } if (f != null && f.hasClass(\"open\")) { return } d.clearTimeout(a); a = d.setTimeout(function () { f.addClass(\"open\") }, b) }).on(\"mouseleave\", function () { d.clearTimeout(a); f = e(this); a = d.setTimeout(function () { f.removeClass(\"open\") }, b) }) })(jQuery, window, 100); \
            //]]> \
        }); \
        function LoadBranding() { \
            $.ajax({ \
                type: \"Post\", \
                url: \"/lus/laboratory-hazards-bulletin/home/branding\", \
                success: function (result) { \
                    $(\"#divWelcomeUser\").html(result); \
                }, \
                error: function () { \
                    $(\"#divWelcomeUser\").html(\"\"); \
                } \
            }); \
        } \
        $(function () { \
            $(window).bind('scroll', function () { \
                if ($(window).scrollTop() > 100) { \
                    $('.scroll-top-wrapper').addClass('show'); \
                } else { \
                    $('.scroll-top-wrapper').removeClass('show'); \
                } \
            }); \
 \
            $('.scroll-top-wrapper').on('click', scrollToTop); \
        }); \
 \
        function scrollToTop() { \
            var element = $('body'); \
            var offset = element.offset(); \
            var offsetTop = offset.top; \
            $('html, body').animate({ scrollTop: offsetTop }, 500, 'linear'); \
        } \
    </script> \
 \
    <title> \
         \
    Laboratory Hazards Bulletin \
 \
 \
    </title> \
         \
    <style> \
        .ac_results { \
            width: 364px !important; \
        } \
    </style> \
 \
     \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/search.js\" type=\"text/javascript\"></script> \
    <script src=\"Laboratory%20Hazards%20Bulletin_files/jquery.js\" type=\"text/javascript\"></script> \
    <link type=\"text/css\" href=\"Laboratory%20Hazards%20Bulletin_files/bootstrap-datepicker.css\" rel=\"stylesheet\"> \
    <link type=\"text/css\" href=\"Laboratory%20Hazards%20Bulletin_files/chosen.css\" rel=\"stylesheet\"> \
    <script type=\"text/javascript\" src=\"Laboratory%20Hazards%20Bulletin_files/chosen.js\"></script> \
    <script type=\"text/javascript\" src=\"Laboratory%20Hazards%20Bulletin_files/bootstrap-datepicker.js\"></script> \
    <script type=\"text/javascript\"> \
        var sessionUrl = '/lus/laboratory-hazards-bulletin/search/createsessionforcitationsdownload'; \
        var suggestionUrl = '/lus/laboratory-hazards-bulletin/suggestion'; \
        var relatedProductUrl = '/lus/laboratory-hazards-bulletin/home/relatedproducts'; \
        var searchResultUrl = '/lus/laboratory-hazards-bulletin/search/getsearchresult?sortby=pubdate'; \
        var searchUrl = '/lus/laboratory-hazards-bulletin/search/search-result'; \
        var facetValueUrl = '/lus/laboratory-hazards-bulletin/Search/GetFacetValues?facet=hazcat'; \
        var quickSearchUrl = '/lus/laboratory-hazards-bulletin/search/quicksearch';     \
         \
        $(document).ready(function () { \
            LoadDataList(\"listOfRelatedProduct\", relatedProductUrl); \
            LoadFacetValues(); \
            suggestion(\"suggestion-class\", suggestionUrl); \
                }); \
         \
    </script> \
     \
     \
 \
</head> \
<body> \
          <!-- Google Tag Manager (noscript) --> \
          <noscript><iframe src=\"https://www.googletagmanager.com/ns.html?id=GTM-KXLLNJR\" height=\"0\" width=\"0\" style=\"display:none;visibility:hidden\"></iframe></noscript> \
          <!-- End Google Tag Manager (noscript) --> \
 \
    <div id=\"header\" class=\"rsc_header header_sec\"> \
 \
<link href=\"Laboratory%20Hazards%20Bulletin_files/museo-sans-500.css\" rel=\"Stylesheet\" type=\"text/css\"> \
 \
<link type=\"text/css\" media=\"screen\" rel=\"stylesheet\" href=\"Laboratory%20Hazards%20Bulletin_files/RSCHeaderFooterNew.css\"> \
 \
<script type=\"text/javascript\"> \
    //<![CDATA[ \
     \
    var _gaq = _gaq || []; \
    _gaq.push(['_setAccount','UA-20361743-1']); \
    _gaq.push(['_setDomainName','.rsc.org']); \
    _gaq.push(['_setAllowLinker', true]); \
    _gaq.push(['_setAllowHash', false]); \
    _gaq.push(['_trackPageview']); \
    _gaq.push(['_trackPageLoadTime']); \
 \
    (function () { \
        var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true; \
        ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js'; \
        var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s); \
    })(); \
 \
 \
    //]]> \
</script> \
  \
<div id=\"screen\" class=\"screen\"> \
    <div id=\"masthead\"> \
        <div class=\"pagewidth\"> \
             \
            <div id=\"mastheader-search-container\"> \
                <form method=\"get\" action=\"http://www.rsc.org/search?ie=&amp;site=Default&amp;output=xml_no_dtd&amp;client=default_frontend&amp;lr=&amp;proxystylesheet=default_frontend&amp;oe=\" novalidate=\"novalidate\"> \
                    <input type=\"text\" class=\"mastheader-search-input anchorinside\" placeholder=\"Search\" tabindex=\"999\" name=\"q\" id=\"txtHeaderSearch\"><input type=\"image\" class=\"mastheader-search-btn\" src=\"Laboratory%20Hazards%20Bulletin_files/btn_search.png\" tabindex=\"1000\"> \
                </form> \
            </div> \
             \
            <ul class=\"masthead-nav\" id=\"masthead-home\"> \
                <li class=\"homepagelink\"><a class=\"\" title=\"Royal Society of Chemistry - Advancing excellence in the chemical sciences\" href=\"http://www.rsc.org/\">Home</a></li> \
                <li class=\"account_container\"> \
                     \
                </li> \
            </ul> \
 \
            <ul class=\"masthead-nav\" id=\"masthead-nav\"> \
 \
                 \
                <li class=\"header_menu_cart \" style=\"display: none;\"> \
                    <div class=\"cart_v4 \"> \
                        <a class=\"fl cart_v4_link\" href=\"https://www.rsc.org/basket/shoppingcart/orderitems?returnurl=%7b1%7d\"> \
                            <span class=\"fr cartItemCount\">0</span></a> \
                    </div> \
                    <div class=\"cart_popup\" style=\"display: none\"> \
                         \
                    </div> \
                </li> \
 \
                 \
                <li><a href=\"http://pubs.rsc.org/\" target=\"_self\" title=\"Fast access to our journals, books and databases\"> \
                    Publishing</a> </li> \
                 \
                <li><a href=\"http://www.chemspider.com/\" target=\"_self\" title=\"Chemical structure search in the free chemical database\"> \
                    ChemSpider</a> </li> \
                 \
                <li><a href=\"http://rsc.org/education\" target=\"_self\" title=\"Education in the Royal Society of Chemistry\"> \
                    Education</a> </li> \
                 \
                <li><a href=\"http://my.rsc.org/\" target=\"_self\" title=\"The online community for chemists\"> \
                    Community</a> </li> \
                 \
                <li><a href=\"http://www.rsc.org/news/index.asp\" target=\"_self\" title=\"Latest news\"> \
                    News</a> </li> \
                 \
                <li class=\"more_link\"><a class=\"masthead-menuclick\" href=\"#\" target=\"_self\" title=\"Links to other areas\" style=\"display: inline-block; height: 15px; line-height: 15px; margin-top: 2px;\"> \
                    More... \
                    <!--[if gte IE 7]><!--> \
                </a> \
                    <!--<![endif]--> \
                    <!--[if lte IE 6]><table><tr><td><![endif]--> \
                    <div id=\"masthead-more-menu\" class=\"more_menu_body_div\" style=\"display: block;\"> \
                        <ul class=\"masthead-menu-body\" id=\"childList\" style=\"display: none;\"> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/aboutus/\" target=\"_self\" title=\"Support us to promote and develop the chemical sciences for the benefit of society\"> \
                                About us</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/chemistryworld/Advertising/index.asp\" target=\"_self\" title=\"Advertise with us\"> \
                                Advertise with us</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://my.rsc.org/careers\" target=\"_self\" title=\"Careers advice, training courses &amp; CPD\"> \
                                Careers service</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/aboutus/chemistrycentre/venue/index.asp\" target=\"_self\" title=\"Member facilities, library opening times &amp; venue hire\"> \
                                The Chemistry Centre (venue hire)</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://jobs.rsc.org/\" target=\"_self\" title=\"Official job board\"> \
                                Chemistry World Jobs</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/chemistryworld/\" target=\"_self\" title=\"Chemistry news, analysis articles, features &amp; opinion\"> \
                                Chemistry World magazine</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://my.rsc.org/chemnet\" target=\"_self\" title=\"Magazine for those aged 16-18 &amp; studying chemistry\"> \
                                ChemNet student networking</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://cssp.chemspider.com/\" target=\"_self\" title=\"Practical and reliable organic, organometallic and inorganic chemical synthesis, reactions and procedures deposited by synthetic chemists\"> \
                                ChemSpider Synthetic Pages</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/conferencesandevents/\" target=\"_self\" title=\"Events in the chemical sciences - listings and highlights\"> \
                                Conferences &amp; events</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/diversity/\" target=\"_self\" title=\"Our approach to supporting diversity in the chemical sciences\"> \
                                Diversity</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/education/\" target=\"_self\" title=\"Activities that cater for chemical scientists of all ages\"> \
                                Educational resources</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/library/\" target=\"_self\" title=\"Chemical structure, business, analysis &amp; chemist information\"> \
                                Library and Information Centre</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/shop/index.asp\" target=\"_self\" title=\"Purchase class-leading products &amp; services\"> \
                                Online shop</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://www.rsc.org/ScienceAndTechnology/Awards/\" target=\"_self\" title=\"Recognition of achievements by individuals, teams and organisations in advancing excellence in the chemical sciences\"> \
                                Prizes &amp; awards</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://mc.manuscriptcentral.com/rsc\" target=\"_self\" title=\"Submit your journal article to us\"> \
                                Submission system (journals)</a></li> \
                             \
                            <li class=\"mobile-only\"> <a class=\"MoreAccessDummyClass\" href=\"http://my.rsc.org/talkchemistry\" target=\"_self\" title=\"Online community for chemistry teachers\"> \
                                Talk Chemistry</a></li> \
                             \
                        </ul> \
                    </div> \
                    <!--[if lte IE 6]></td></tr></table></a><![endif]--> \
                </li> \
                 \
            </ul> \
        </div> \
    </div> \
</div> \
 \
<noscript> \
    <style> \
        li.more_link:hover .more_menu_body_div \
        { \
            visibility: visible !important; \
            display: block !important; \
            background-color: #272521; \
        } \
 \
        li.more_link:hover .masthead-menu-body \
        { \
            visibility: visible !important; \
            display: block !important; \
        } \
 \
 \
 \
        li.account_container:hover div.account_popup \
        { \
            visibility: visible !important; \
            display: block !important; \
        } \
 \
        li.li_cart:hover div.cart_popup \
        { \
            visibility: visible !important; \
            display: block !important; \
        } \
    </style> \
</noscript> \
<script type=\"text/javascript\"> \
    var iGlobalMoreMenuOpen = false; \
    var iGlobalWindowWidth = 0; \
    var iGlobalNavPos = 0; \
    var browserIsIE = false; \
    var isuserMenuOpen = false; \
    var isbasketOpen = false; \
    document.getElementById(\"childList\").style.display=\"none\"; \
    var imenustate = 0;var imenuexpand=0; \
    var menuClicked = 0;var menuExpanded=0; \
    var dynamicQueryLoaded = false; \
    if (typeof jQuery == \"undefined\" || !$.isFunction($.fn.slideToggle)) { \
        var script_tag = document.createElement(\"script\"); \
        script_tag.setAttribute(\"type\", \"text/javascript\"); \
        script_tag.setAttribute(\"src\", \"http://www.rsc-cdn.org/www.rsc.org/api-v2/content/stylesheets/headerfooter/jquery-1.7.2.js?3.7.1.0\") \
        script_tag.onload = Main; // Run main() once jQuery has loaded \
        script_tag.onreadystatechange = function () { // Same thing but for IE \
            if (this.readyState == \"complete\" || this.readyState == \"loaded\") { \
 \
                Main(); \
            } \
        } \
        document.getElementsByTagName(\"head\")[0].appendChild(script_tag); \
    } \
 \
         \
 \
    function loadSimleModal() { \
        var src = \"http://www.rsc-cdn.org/www.rsc.org/api-v2/scripts/simplemodal.js?3.7.1.0\"; \
        var script_tag = document.createElement(\"script\"); \
        script_tag.setAttribute(\"type\", \"text/javascript\"); \
        script_tag.setAttribute(\"src\", src); \
        document.getElementsByTagName(\"head\")[0].appendChild(script_tag); \
    } \
 \
    function Main() { \
 \
        LoadPlaceHolder();  \
        if(!dynamicQueryLoaded) \
        {             \
 \
             \
            dynamicQueryLoaded = true; \
            $(\"ul.header_lw_level_2\").attr('class', 'header_lw_level_21 header_menu_dd_container'); \
            $(\"ul.header_lw_level_21\").hide(); \
         \
            iGlobalWindowWidth = $(window).width(); \
            $(\"#masthead-more-menu\").show(); \
            iGlobalNavPos = $(\"#masthead-nav\").offset().left + $(\"#masthead-nav\").width() - 60; \
          \
       \
            function Handlekeydown($item,e,mode)   \
            { \
       \
                if (e.altKey || e.ctrlKey) { \
                    // Modifier key pressed: Do not process \
                    return true; \
                } \
 \
                switch (e.keyCode) { \
                    case 9: //tab                \
                    case 27: $item.parent().parent().parent().parent().find('a.anchorinside').focus(); \
                    case 32: //space \
                        { \
                            ShowMenu(); \
                            var $itemUL = $item.parent().parent().parent().parent(); \
                    \
                            // move up one level \
                            //$activeItem = $itemUL.children(':first-child');                      \
                              $activeItem = $itemUL; \
 \
                            // set focus on the new item \
                            $activeItem.focus();                 \
 \
                            e.stopPropagation(); \
                            return false; \
                            break; \
                        }            \
                    case 38: //up \
                        {                 \
                            if( $item.parent().prev().length >0) \
                            { \
                                $item.parent().prev().find('>a').focus(); \
                            } \
                            else \
                            { \
                                $item.parent().parent().children().last().find('>a').focus(); \
                            } \
                            return false; \
                            break; \
                    \
                        } \
                    case 40: //down \
                        {                       \
                            // $item.siblings().first().focus(); \
                            if( $item.parent().next().length >0) \
                            { \
                                $item.parent().next().find('>a').focus(); \
                            } \
                            else \
                            { \
                                //$item.parent().children().first().focus(); \
                                $item.parent().parent().children().first().find('>a').focus(); \
                            } \
 \
                            return false; \
                            break; \
 \
                        } \
 \
                    case 13: \
                        { \
                            e.stopPropagation(); \
                            return true; \
                            break; \
                        } \
                } // end switch \
 \
                return true; \
 \
            } \
 \
              $('.masthead-menuclick, .masthead-menuclick-mobile').click(function () \
            { \
                return ShowMenu(); \
		 \
            }); \
 \
            function  ShowMenu()    { \
                if(iGlobalMoreMenuOpen) \
                { \
                    iGlobalMoreMenuOpen = false; \
                } else { \
                    iGlobalMoreMenuOpen = true;			 \
                } \
                $('#masthead-more-menu ul').slideToggle('medium').css({'display':'inline-block'}); \
		 \
                if (iGlobalNavPos + 300 > iGlobalWindowWidth)  \
                { \
                    iGlobalNavPos = iGlobalWindowWidth - 300; \
                } \
                //iGlobalNavPos=403; \
                iGlobalNavPos2='auto'; \
                $('#masthead-more-menu').css(\"left\", iGlobalNavPos2); \
                $('#masthead-more-menu').css(\"top\", \"38px\"); \
		 \
                if ($.browser && $.browser.msie && parseInt($.browser.version, 10) == 7) { \
                    $('#masthead-more-menu').css(\"right\", \"-236px\"); \
                } \
		 \
                return false; \
            } \
     \
            $(\".header_swipe_dd\").click(function(){ \
                $(\".header_lw_dd_list\").slideToggle('slow'); \
                menuExpanded=0; \
                if (imenuexpand == 0) { \
                    imenuexpand = 1; \
                } \
                else { \
                    imenuexpand = 0; \
                } \
                return false; \
            }); \
 \
            $(document).click(function () {         \
                if(iGlobalMoreMenuOpen) \
                { \
                    $('#masthead-more-menu ul').slideUp('medium'); \
                    iGlobalMoreMenuOpen = false; \
                } \
 \
                if(isuserMenuOpen == true) \
                { \
                    $('.account_popup').slideUp('medium'); \
                    isuserMenuOpen = false; \
                } \
 \
                if(isbasketOpen == true) \
                { \
                    $('.cart_popup').slideUp('medium'); \
                    isbasketOpen = false; \
                } \
 \
                $(\".loggedin_options\").hide(); \
            }); \
             \
             \
            InitializeLightboxLogin(); \
        } \
        \
 \
        $('.basket').click(function(e){          \
            return ShowCartPopup(); \
        }); \
         \
 \
        function ShowCartPopup() \
        { \
            iGlobalNavPos2='auto'; \
            $('.cart_popup').css(\"left\", \"5px\"); \
		 \
            if(isbasketOpen) \
                isbasketOpen = false; \
            else \
                isbasketOpen = true; \
            if ($.browser && $.browser.msie && parseInt($.browser.version, 10) == 7) { \
                $('.cart_popup').css(\"right\", \"-236px\"); \
            } \
            $('.cart_popup').slideToggle('medium').css({'display':'inline-block'}); \
            return false; \
        } \
 \
         \
         \
        $('.user_logged_in').click(function(e){ \
         \
            return ShowLoggedinMenu(); \
       \
        });	 \
                \
       \
 \
        function ShowLoggedinMenu() \
        { \
            iGlobalNavPos2='auto'; \
            $('.account_popup').css(\"left\", \"3px\"); \
		 \
            if(isuserMenuOpen) \
                isuserMenuOpen = false; \
            else \
                isuserMenuOpen = true; \
            if ($.browser && $.browser.msie && parseInt($.browser.version, 10) == 7) { \
                $('.account_popup').css(\"right\", \"-236px\"); \
            } \
            $('.account_popup').slideToggle('medium').css({'display':'inline-block'}); \
            return false; \
        } \
 \
    	 \
	 \
    } \
	 \
    var isDisplayBasketAlways = false; \
      \
    function SetOrderItemCount(data) \
    { \
     \
        var basketCount = parseInt(data); \
        if(basketCount < 0) \
        { \
            basketCount = 0; \
        } \
 \
 \
        if(basketCount > 0 || (basketCount == 0 && isDisplayBasketAlways)) \
        { \
            $(\".header_menu_cart\").show(); \
                                 \
            $(\".cartItemCount\").html(data); \
        } \
        else \
        { \
            $(\".header_menu_cart\").hide(); \
        } \
    } \
 \
    function GetOrderItemCount() \
    { \
         \
        var ec_ot = getCookie(\"BasketCookieGuid\"); \
        var mg_s_t = '', getCountUrl = '' ; \
        var userDetails; \
        userDetails = getCookie(\"ud\"); \
                if (typeof ec_ot === 'undefined') { \
				} \
				else \
				{ \
        $.ajax({ \
            type:\"GET\", \
            url: \"https://www.rsc.org/basket/shoppingcart/getorderitemcount\", \
            data:\"orderToken=\" + ec_ot + \"&isAjaxCall=True&registeredurl=\", \
            cache: false, \
            async: false, \
            crossDomain: true, \
            dataType: 'jsonp', \
            jsonpCallback: \"SetOrderItemCount\", \
            success: function(html){ \
                var basketCount = parseInt(html); \
                if(basketCount < 0) \
                { \
                    basketCount = 0; \
                } \
                         \
                if(basketCount > 0 || (basketCount == 0 && isDisplayBasketAlways)) \
                    $(\".header_menu_cart\").show(); \
                                 \
                $(\".cartItemCount\").html(html); \
                     \
            } \
        }); \
    } \
    } \
 \
    function redirectTo(url) { \
        window.location.href = url; \
    } \
 \
    function loginClickHandler(e) { \
        //ensure that we have jquery and the modal pluign loaded before attempting to do anything with the lightbox \
        if ($ && $.fn.modal) { \
            e.preventDefault(); \
            targetUrl = this.href; \
            getLoginStatus(); \
        } \
    } \
 \
    function getLoginStatus() { \
        jsonpSuccess = false; \
        var src = 'https://www.rsc.org/rsc-id/Account/CheckIfUserIsLoggedIn' + \"?callback=processLoginStatus\"; \
        var script_tag = document.createElement(\"script\"); \
        script_tag.setAttribute(\"type\", \"text/javascript\"); \
        script_tag.setAttribute(\"src\", src); \
        document.getElementsByTagName(\"head\")[0].appendChild(script_tag); \
 \
        setTimeout(function () { \
            if (!jsonpSuccess) { redirectTo(targetUrl); } \
        }, 10000); // assuming 10sec is the max wait time for results \
    } \
 \
    function processLoginStatus(status) { \
        jsonpSuccess = true; \
        if (status.LoggedIn) { \
            redirectTo(targetUrl); \
        } \
        else { \
            displayLoginLightbox(targetUrl); \
        } \
    } \
 \
    function InitializeLightboxLogin() { \
         \
    } \
     \
    function displayLoginLightbox(targetUrl) { \
        var src = targetUrl + \"&lightbox=true\"; \
 \
        function setSrc() { \
            $('#frmLightboxLogin').attr(\"src\", src); \
        } \
 \
        $.modal('<iframe id=\"frmLightboxLogin\" src=\"\" height=\"400\" width=\"900\" style=\"border:0\" scrolling=\"no\" allowTransparency=\"true\" marginheight=\"0\" marginwidth=\"0\" frameborder=\"0\">', { \
            onShow: function (dialog) { \
                setSrc(); \
            }, \
            closeHTML: \"\", \
            containerCss: { \
                padding: 0, \
                width: 900 \
            }, \
            overlayClose: false \
        }); \
    } \
 \
    function parseData(str) { \
        var parsed = {}; \
        var pairs = str.split(\"||\"); \
        for (var i = 0, len = pairs.length, keyVal; i < len; ++i) { \
            keyVal = pairs[i].split(\"==\"); \
            if (keyVal[0]) { \
                parsed[keyVal[0]] = keyVal[1]; \
            } \
        } \
        return parsed; \
    } \
 \
    function receiveMessage(event) { \
        var data = parseData(event.data); \
        if (data.message == \"LOGIN_SUCCESSFUL\") { \
            $.modal.close(); \
            window.location.href = decodeURIComponent(data.location); \
        } else if (data.message == \"CLOSE_LIGHTBOX\") { \
            $.modal.close(); \
        } else if (data.message == \"RESIZE\") { \
            var f = document.getElementById(\"frmLightboxLogin\"); \
            if(f) { \
                f.style.height = data.requiredHeight + 'px'; \
            } \
        } else { \
            if (data.location) { \
                window.location.href = decodeURIComponent(data.location); \
            } \
        } \
    } \
 \
    function getCookie(c_name) \
    { \
        var c_value = document.cookie; \
        var c_start = c_value.indexOf(\" \" + c_name + \"=\"); \
        if (c_start == -1) \
        { \
            c_start = c_value.indexOf(c_name + \"=\"); \
        } \
        if (c_start == -1) \
        { \
            c_value = null; \
        } \
        else \
        { \
            c_start = c_value.indexOf(\"=\", c_start) + 1; \
            var c_end = c_value.indexOf(\";\", c_start); \
            if (c_end == -1) \
            { \
                c_end = c_value.length; \
            } \
            c_value = unescape(c_value.substring(c_start,c_end)); \
        } \
        return c_value; \
    } \
 \
    function RedirectToeCommerce() \
    { \
        window.location.href = 'https://www.rsc.org/basket/shoppingcart/orderitems?returnurl={0}'; \
    } \
 \
    function RedirectToeCommerceCheckOut() \
    { \
        window.location.href ='https://www.rsc.org/basket/shoppingcart/redirectworkflowtoaction?from=cart&amp;returnurl={0}'; \
    } \
 \
 \
    if(typeof $ != \"undefined\") \
    { \
        $(document).ready(function () { \
            if (typeof jQuery != \"undefined\" || !$.isFunction($.fn.slideToggle)) { \
                $(\"ul.header_lw_level_2\").hide();   \
                Main();        \
            } \
        });     \
    } \
 \
 \
    function LoadPlaceHolder() { \
        //     \
        if (('placeholder' in document.createElement('input')) === false) { \
       \
            $('[placeholder]').focus(function () { \
                var input = $(this); \
                if (isPlaceholder(input)) { \
                    input.val(''); \
                    input.removeClass('placeholder'); \
                } \
            }).blur(function () { \
                var input = $(this); \
                if (input.val() == '' || isPlaceholder(input)) { \
                    input.addClass('placeholder'); \
                    input.val(input.attr('placeholder')); \
                } \
            }).blur().parents('form').submit(function () { \
                $(this).find('[placeholder]').each(function () { \
                    var input = $(this); \
                    if (isPlaceholder(input)) { \
                        input.val(''); \
                    } \
                }) \
            }); \
        } \
    } \
    //    }); \
 \
    // Convenience function to check if elem's value is equal to its placeholder attribute.  \
    // Only used for browsers which don't support placeholder \
    function isPlaceholder(elem) { \
        return elem.val() == elem.attr('placeholder'); \
    } \
		 \
</script> \
 \
<script type=\"text/javascript\"> \
 \
     \
</script> \
 \
    </div> \
    <div class=\"container\"> \
        <div class=\"logo-header\"> \
            <div class=\"logo-block\"> \
                <div class=\"col-xs-6 col-sm-6 col-md-4 pull-left margin-top17 no-padding-left\"> \
                    <a href=\"http://pubs.rsc.org/\"> \
                        <img src=\"Laboratory%20Hazards%20Bulletin_files/publishing-logo.png\" alt=\"Publishing Logo\"> \
                    </a> \
                </div> \
                <span class=\"divWelcomeUser hidden-tab hidden-xs\" id=\"divWelcomeUser\">    <div class=\"institute-logo-container\"> \
        <div style=\"vertical-align: middle; text-align: center; margin: 20px 0px auto; font-weight: bold; display: table-cell; color: #000;padding-top:20px;font-size:12px;\"> \
            Capes \
        </div> \
    </div> \
</span> \
                <div class=\"col-xs-3 col-sm-3 col-md-8 pull-right margin-top10\"> \
                    <a href=\"http://rsc.org/\" title=\"Royal Society of Chemistry homepage\" class=\"pull-right ie8logofix hidden-xs\"> \
                        <img src=\"Laboratory%20Hazards%20Bulletin_files/rsc-logo-v-180.png\" title=\"Royal Society of Chemistry homepage\" alt=\"Royal Society of Chemistry homepage\"> \
                    </a> \
                </div> \
            </div> \
        </div> \
        <div class=\"row\"> \
            <div class=\"col-md-12 no-padding\"> \
                <div class=\"navbar navbar-inverse navbar-static-top navbar-site \" role=\"navigation\" id=\"pubsMenu\"> \
    <div class=\"\"> \
        <div class=\"navbar-header\"> \
            <button type=\"button\" class=\"navbar-toggle\" data-toggle=\"collapse\" data-target=\".navbar-collapse\"> \
                <span class=\"sr-only\">Toggle navigation</span> \
                <span class=\"icon-bar\"></span> \
                <span class=\"icon-bar\"></span> \
                <span class=\"icon-bar\"></span> \
            </button> \
        </div> \
        <div class=\"navbar-collapse collapse\"> \
            <ul class=\"nav navbar-nav nav-site\"> \
                    <li class=\"dropdown\"> \
                        <a target=\"_self\" class=\"dropdown-toggle\" data-toggle=\"dropdown\" title=\"\" href=\"http://pubs.rsc.org/en/journals?key=title&amp;value=current\">Journals \
                                <b class=\"caret2\"></b> \
                        </a> \
                            <ul class=\"dropdown-menu no-padding\"> \
                                    <li><a target=\"_self\" title=\"\" href=\"http://pubs.rsc.org/en/journals?key=title&amp;value=current\">Current</a></li> \
                                    <li><a target=\"_self\" title=\"\" href=\"http://pubs.rsc.org/en/journals?key=title&amp;value=archive\">Archives</a></li> \
                                    <li><a target=\"_self\" title=\"\" href=\"http://pubs.rsc.org/en/journals?key=title&amp;value=all\">All</a></li> \
                            </ul> \
                    </li> \
                    <li class=\"dropdown\"> \
                        <a target=\"_self\" class=\"dropdown-toggle\" data-toggle=\"dropdown\" title=\"\" href=\"http://pubs.rsc.org/en/ebooks?key=title&amp;value=newtitle\">Books \
                                <b class=\"caret2\"></b> \
                        </a> \
                            <ul class=\"dropdown-menu no-padding\"> \
                                    <li><a target=\"_self\" title=\"\" href=\"http://pubs.rsc.org/en/ebooks?key=title&amp;value=newtitle\">New titles</a></li> \
                                    <li><a target=\"_self\" title=\"\" href=\"http://pubs.rsc.org/en/ebooks?key=title&amp;value=all\">All</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://pubs.rsc.org/bookshop\">Bookshop</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://pubs.rsc.org/en/pickandchoose/login\">Pick and Choose</a></li> \
                            </ul> \
                    </li> \
                    <li class=\"dropdown\"> \
                        <a target=\"_self\" class=\"dropdown-toggle\" data-toggle=\"dropdown\" title=\"\" href=\"http://www.rsc.org/publishing/currentawareness/index.asp\">Databases \
                                <b class=\"caret2\"></b> \
                        </a> \
                            <ul class=\"dropdown-menu no-padding\"> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/journals-books-databases/databases-literature-updates/#literature-updating-services\">Literature updates</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.chemspider.com/\">ChemSpider</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/merck-index\">The Merck Index*</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://pubs.rsc.org/marinlit\">MarinLit</a></li> \
                            </ul> \
                    </li> \
                    <li class=\"dropdown\"> \
                        <a target=\"_blank\" class=\"dropdown-toggle\" data-toggle=\"dropdown\" title=\"\" href=\"http://www.rsc.org/publishing/journals/forms/profile.asp\">Alerts \
                                <b class=\"caret2\"></b> \
                        </a> \
                            <ul class=\"dropdown-menu no-padding\"> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/publishing/journals/forms/profile.asp\">Subscribe</a></li> \
                                    <li><a target=\"_self\" title=\"\" href=\"http://pubs.rsc.org/en/ealerts/rssfeed\">RSS Feeds</a></li> \
                            </ul> \
                    </li> \
                    <li class=\"dropdown\"> \
                        <a target=\"_self\" class=\"dropdown-toggle\" data-toggle=\"dropdown\" title=\"\" href=\"http://www.rsc.org/Advertising/index.asp\">Other \
                                <b class=\"caret2\"></b> \
                        </a> \
                            <ul class=\"dropdown-menu no-padding\"> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/Advertising/index.asp\">Advertising &amp; Partnerships</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://blogs.rsc.org/\">Blogs</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/chemistryworld\">Chemistry World</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/education/eic/index.asp\">Education in Chemistry </a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/Publishing/Journals/OpenScience/index.asp\">Open Access</a></li> \
                            </ul> \
                    </li> \
                    <li class=\"dropdown\"> \
                        <a target=\"_self\" class=\"dropdown-toggle\" data-toggle=\"dropdown\" title=\"\" href=\"http://pubs.rsc.org/en/home/faq\">Help \
                                <b class=\"caret2\"></b> \
                        </a> \
                            <ul class=\"dropdown-menu no-padding\"> \
                                    <li><a target=\"_self\" title=\"\" href=\"http://pubs.rsc.org/en/home/faq\">FAQ</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/aboutus/contacts/\">Contact us</a></li> \
                                    <li><a target=\"_blank\" title=\"\" href=\"http://www.rsc.org/publishing/aboutrscpublishing.asp\">About RSC Publishing</a></li> \
                            </ul> \
                    </li> \
            </ul> \
        </div> \
    </div> \
 \
                </div> \
            </div> \
        </div> \
        <div class=\"row\"> \
 \
            <div class=\"col-md-12 row-pad-10 container-wrap\"> \
                <ol class=\"breadcrumb\"> \
                    <li><a href=\"http://pubs.rsc.org/\" title=\"Home\">Home</a></li> \
                    <li><a href=\"http://www.rsc.org/publishing/currentawareness/index.asp\" title=\"Databases\">Databases</a></li> \
                     \
    <li class=\"active\">Laboratory Hazards Bulletin</li> \
 \
                     \
                </ol> \
            </div> \
        </div> \
        <div class=\"row container-wrap row-pad-10 padding-top10\"> \
            <div class=\"col-md-12 col-sm-12 col-xs-12 no-padding\"> \
                <div class=\"lightGrey-bg fl full-size\"> \
                    <span class=\"chemistryCollectionTlogo\"> \
                        <a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/\"> \
 \
                            <img src=\"Laboratory%20Hazards%20Bulletin_files/LHB_logo.png\" alt=\"Laboratory Hazards Bulletin\" title=\"Laboratory Hazards Bulletin\" class=\"img-responsive\"> \
                        </a> \
                        <span> \
                             \
                        </span> \
                    </span> \
                </div> \
            </div> \
        </div> \
    </div> \
    <div class=\"container\"> \
 \
<script type=\"text/javascript\"> \
    //<![CDATA[   \
    var addthis_config = { \
        data_track_clickback: false \
    } \
    $(document).ready(function () {         \
        $(\".emailPopup\").click(function () { \
            window.open(this.href, 'EmailWindow', 'location=0,status=1,scrollbars=1,width=540,height=600,toolbar=0,directories=0,resizable=0,menubar=0'); \
            return false; \
        }); \
 \
    }); \
    //]]> \
</script> \
 \
 \
<div class=\"social_bar hidden-xs hidden-sm\" style=\"margin-left: 985px;\"> \
    <div class=\"responsive\"> \
        <a href=\"http://www.addthis.com/bookmark.php?v=250&amp;pub=xa-4a7c13c424a726f2\" onmouseout=\"addthis_close()\" onclick=\"return addthis_sendto()\" class=\"social_style_share addthis_button\"><span class=\"bookmark_icon\"></span></a><a class=\"social_style_email emailPopup\" title=\"Email this page\" target=\"_blank\" id=\"lnkEmail\" href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/email/email?url=http%3A%2F%2Fpubs.rsc.org%3A8032%2Flus%2Flaboratory-hazards-bulletin\" rel=\"nofollow\" onclick=\"_gaq.push(['_trackSocial', 'email', 'email', '/lus/laboratory-hazards-bulletin']);\">&nbsp;</a> \
            <span style=\"display: inline-block\"><a class=\"social_icon social_style_facebook addthis_button_facebook\" style=\"float: left;\" onclick=\"_gaq.push(['_trackSocial', 'Facebook', 'like', '/lus/laboratory-hazards-bulletin']);\"></a></span> \
            <span style=\"display: inline-block\"><a class=\"social_icon social_style_twitter addthis_button_twitter\" style=\"float: left;\" onclick=\"_gaq.push(['_trackSocial', 'Twitter', 'tweet', '/lus/laboratory-hazards-bulletin']);\"></a></span> \
            <span style=\"display: inline-block\"><a class=\"social_icon social_style_google_plusone addthis_button_google_plusone\" g:plusone:count=\"false\" style=\"float: left;\" onclick=\"_gaq.push(['_trackSocial', 'Google Plus', 'plus', '/lus/laboratory-hazards-bulletin']);\"></a></span> \
            <script type=\"text/javascript\"> \
                var addthis_config = { \
                    data_ga_property: 'UA-20361743-1', \
                    data_ga_social: true \
                }; \
            </script> \
 \
 \
 \
    </div> \
    <a title=\"Feedback\" class=\" social_feedback_image social_icons_feedback feedbackclick\" href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/email/feedback\">&nbsp;</a> \
     \
</div> \
<div id=\"divFeedbackPopUp\" class=\"feedbackpopupcontent\" style=\"display: none; width: 498px;\"> \
    <div id=\"divLoadingFeedback\"> \
        <img alt=\"Loading\" style=\"display: block\" src=\"Laboratory%20Hazards%20Bulletin_files/ajax-loader.gif\"> \
    </div> \
</div> \
<div id=\"divNewsletterPopup\" class=\"newsletterpopupcontent\" style=\"display: none; width: 498px;\"> \
    <div id=\"divLoadingNewsletter\"> \
        <img alt=\"Loading\" style=\"display: block\" src=\"Laboratory%20Hazards%20Bulletin_files/ajax-loader.gif\"> \
    </div> \
</div> \
 \
<input id=\"SubmitFeedbackUrl\" name=\"SubmitFeedbackUrl\" type=\"hidden\" value=\"/lus/laboratory-hazards-bulletin/email/feedbackdata\"> \
 \
<script type=\"text/javascript\"> \
    //<![CDATA[    \
 \
    $(window).resize(function () { \
 \
        if ($(window).width() <= 1040) { \
            //$(\".social_bar\").css(\"display\", \"none\"); \
            $(\".scroll-top-wrapper\").css(\"margin-left\", \"\").css(\"right\", \"0\"); \
            $(\".social_bar\").css(\"margin-left\", \"\").css(\"right\", \"0\"); \
        } \
        else { \
            $(\".scroll-top-wrapper\").css(\"margin-left\", \"985px\").css(\"right\", \"\"); \
            $(\".social_bar\").css(\"margin-left\", \"985px\").css(\"right\", \"\"); \
            \
        } \
    }); \
    $(window).load(function () { \
        if ($(window).width() <= 1040) { \
            $(\".scroll-top-wrapper\").css(\"margin-left\", \"\").css(\"right\", \"0\"); \
            //$(\".social_bar\").css(\"display\", \"none\"); \
            $(\".social_bar\").css(\"margin-left\", \"\").css(\"right\", \"0\"); \
        } \
        else { \
            $(\".scroll-top-wrapper\").css(\"margin-left\", \"985px\").css(\"right\", \"\"); \
            $(\".scroll-top-wrapper\").css(\"*left\", \"220px !important\").css(\"right\", \"\"); \
            $(\".social_bar\").css(\"margin-left\", \"985px\").css(\"right\", \"\"); \
            $(\".social_bar\").css(\"*left\", \"220px !important\").css(\"right\", \"\"); \
        } \
    }); \
     \
    $(document).ready(function () { \
        var urlSubmitNewsletter = $('#SubmitNewsletterUrl').val(); \
        var urlSubmitFeedback = $('#SubmitFeedbackUrl').val(); \
         \
        $('.feedbackclick').click(function (event) { \
            event.preventDefault(); \
            LoadAgain(urlSubmitFeedback, '#divFeedbackPopUp', '#FeedbackSubmit', '.hinttextboxfeedback', 'F'); \
            var feedbackpopupid = $('#divFeedbackPopUp'); \
            var fadefeedbackdivid = \"#fadeFeedbackwidget\"; \
            if ($.browser.msie && $.browser.version == '6.0') \
                $(feedbackpopupid).css(\"position\", \"absolute\"); \
            else \
                $(feedbackpopupid).css(\"position\", \"fixed\"); \
            $(feedbackpopupid).css(\"top\", \"140px\"); \
            $.browser.chrome = /chrome/.test(navigator.userAgent.toLowerCase()); \
            if ($.browser.chrome) \
                $(feedbackpopupid).css(\"top\", \"140px\"); \
            if ($.browser.msie && $.browser.version == '9.0') \
                $(feedbackpopupid).css(\"top\", \"140px\"); \
            if ($.browser.msie && $.browser.version == '7.0') \
                $(feedbackpopupid).css(\"top\", \"140px\"); \
            if ($(window).width() <= 1040) { \
                $(feedbackpopupid).css(\"right\", \"30px\").css(\"left\", \"\"); \
            } \
            else { \
                var left = (($(window).width() - 980) / 2) + 480; \
                $(feedbackpopupid).css(\"right\", \"\").css(\"left\", left + \"px\"); \
            } \
            if ($(window).height() <= 600) { \
                $(feedbackpopupid).css(\"top\", \"90px\"); \
            } \
            $(feedbackpopupid).fadeIn(); \
            $('body').append('<div id=\"fadeFeedbackwidget\"></div>'); \
            $(fadefeedbackdivid).css({ 'filter': 'alpha(opacity=0)' }).fadeIn(); \
            if ($.browser.msie && $.browser.version.substr(0, 1) < 7) { \
                $(fadefeedbackdivid).css(\"height\", $(document).height()); \
                $(fadefeedbackdivid).css(\"width\", $(document).width() - 20); \
            } \
            else { \
                $(fadefeedbackdivid).css(\"height\", '100%'); \
                $(fadefeedbackdivid).css(\"width\", '100%'); \
            } \
            $('.social_feedback_image').removeClass('social_icons_feedback').addClass('social_icons_feedback_selected'); \
        }); \
 \
        $('.newsletterclick').click(function (event) { \
            event.preventDefault(); \
            LoadAgain(urlSubmitNewsletter, '#divNewsletterPopup', '#NewsLetterSubmit', '.hinttextboxNews', 'N'); \
            var newsLetterpopupid = $('#divNewsletterPopup'); \
            var fadenewsletterdivid = \"#fadeNewsletterwidget\"; \
            if ($.browser.msie && $.browser.version == '6.0') \
                $(newsLetterpopupid).css(\"position\", \"absolute\"); \
            else \
                $(newsLetterpopupid).css(\"position\", \"fixed\"); \
 \
 \
            $(newsLetterpopupid).css(\"top\", \"140px\"); \
            $.browser.chrome = /chrome/.test(navigator.userAgent.toLowerCase()); \
            if ($.browser.chrome) \
                $(newsLetterpopupid).css(\"top\", \"140px\"); \
            if ($.browser.msie && $.browser.version == '9.0') \
                $(newsLetterpopupid).css(\"top\", \"140px\"); \
            if ($.browser.msie && $.browser.version == '7.0') \
                $(newsLetterpopupid).css(\"top\", \"140px\"); \
            if ($(window).width() <= 1040) { \
                $(newsLetterpopupid).css(\"right\", \"30px\").css(\"left\", \"\"); \
            } \
            else { \
                var left = (($(window).width() - 980) / 2) + 480; \
                $(newsLetterpopupid).css(\"right\", \"\").css(\"left\", left + \"px\"); \
            } \
            if ($(window).height() <= 600) { \
                $(newsLetterpopupid).css(\"top\", \"90px\"); \
            } \
            $(newsLetterpopupid).fadeIn(); \
            $('body').append('<div id=\"fadeNewsletterwidget\"></div>'); \
            $(fadenewsletterdivid).css({ 'filter': 'alpha(opacity=0)' }).fadeIn(); \
            if ($.browser.msie && $.browser.version.substr(0, 1) < 7) { \
                $(fadenewsletterdivid).css(\"height\", $(document).height()); \
                $(fadenewsletterdivid).css(\"width\", $(document).width() - 20); \
            } \
            else { \
                $(fadenewsletterdivid).css(\"height\", '100%'); \
                $(fadenewsletterdivid).css(\"width\", '100%'); \
            } \
            $('.social_newsletter_image').removeClass('social_icons_newsletter').addClass('social_icons_newsletter_selected'); \
        }); \
 \
 \
        function fadeoutFeedbackpopup() { \
            $('#fadeFeedbackwidget,#divFeedbackPopUp').fadeOut(function () { \
                $('#fadeFeedbackwidget').remove(); \
            }); \
            $('.social_feedback_image').removeClass('social_icons_feedback_selected').addClass('social_icons_feedback'); \
        } \
        function fadeoutNewsletterpopup() { \
            $('#fadeNewsletterwidget,#divNewsletterPopup').fadeOut(function () { \
                $('#fadeNewsletterwidget').remove(); \
            }); \
            $('.social_newsletter_image').removeClass('social_icons_newsletter_selected').addClass('social_icons_newsletter'); \
        } \
        $('.close_feedback').live(\"click\", function (event) { \
            event.preventDefault(); \
            fadeoutFeedbackpopup(); \
        }); \
        $('.close_newsletter').live(\"click\", function (event) { \
            event.preventDefault(); \
            fadeoutNewsletterpopup(); \
        }); \
        function LoadAgain(formname, formdiv, popupstate, hinttextbox, formTyp) { \
            $.ajax({ \
                type: \"Get\", \
                url: formname, \
                error: function (result) { \
                    $(formdiv).html(result); \
                    // $(hinttextbox).hint(); \
                    $(popupstate).val() == \"0\"; \
                }, \
                success: function (result) { \
                    $(formdiv).html(result); \
                    // $(hinttextbox).hint(); \
                    $(popupstate).val() == \"0\"; \
                    if (formTyp == 'N') { \
                        $(\"#newsletterCancel\").remove(); \
                        $(\".delete_button_newsletter\").css(\"float\", \"right\"); \
                        $(\".delete_button_newsletter\").show(); \
                    } \
                    if (formTyp == 'F') { \
                        $(\"#feedbackCancel\").remove(); \
                        $(\".delete_button_feedback\").css(\"float\", \"right\"); \
                        $(\".delete_button_feedback\").show(); \
                    } \
                } \
            }); \
        } \
 \
        $('#fadeNewsletterwidget').live(\"click\", function () { \
            fadeoutNewsletterpopup(); \
        }); \
        $('#fadeFeedbackwidget').live(\"click\", function () { \
            fadeoutFeedbackpopup(); \
        }); \
 \
        $('.bookmark_icon').hover(function () { \
            $('#at15s').css({ 'position': 'fixed' }); \
        }); \
 \
        if ($.browser.msie && $.browser.version == \"6.0\") { \
            try { \
                document.execCommand(\"BackgroundImageCache\", false, true); \
            } catch (err) { } \
        } \
    }); \
 \
    //]]> \
</script> \
 \
        <div class=\"scroll-top-wrapper hidden-xs hidden-sm\" title=\"Top\" style=\"margin-left: 985px;\"> \
            <span class=\"scroll-top-inner\"> \
                <i class=\"fa fa-2x fa-arrow-circle-up\"></i> \
            </span> \
        </div> \
    </div> \
     \
 \
 \
<div class=\"container\"> \
    <div class=\"row container-wrap row-pad-10 text-pad\"> \
        <div class=\"col-md-12 col-sm-12 col-xs-12\"> \
            <div class=\"desc desc-margin-bottom0\"> \
                Laboratory Hazards Bulletin provides you with details of \
 literature reports on safety measures, potential hazards and new  \
legislation affecting workers in laboratories. Covering a wide range of  \
primary sources, including key scientific and trade journals, you can  \
search records according to subject and chemical keywords, filter your  \
reading to your subject area of choice or simply browse recently added  \
articles. \
            </div> \
        </div> \
    </div> \
    <div class=\"row container-wrap row-pad-10\"> \
        <div class=\"clearfix search-box\"> \
 \
            <div class=\"col-sm-12 col-md-9 col-xs-12 padding-left0 padding-right10 sample-right-pad0 padding-top10-sx\"> \
<div class=\"search-box-div\"> \
    <div class=\"search\"> \
        <h3 class=\"search-heading\">Search \
        </h3> \
        <div class=\"divContextualMessage\" style=\"display: none; left: 440.5px;\"> \
            <div class=\"contextual_topImage\"> \
                <img src=\"Laboratory%20Hazards%20Bulletin_files/contextual_help_top.gif\" title=\"help\" alt=\"contextual help image\"> \
            </div> \
            <div class=\"contextual_middleImage\"> \
                <div class=\"inner_msg\"> \
                    Search Help \
                </div> \
            </div> \
            <div class=\"contextual_BottomImage\"> \
                &nbsp; \
            </div> \
        </div> \
    </div> \
    <div class=\"clear\"> \
        <div class=\"col-sm-8 col-md-8 col-xs-12 pull-left padding-right0 padding-left0\"> \
            <input class=\"full-size no-padding search-text-box ac_input suggestion-class\" id=\"freeSearch\" name=\"freeSearch\" placeholder=\"Quick search\" title=\"Quick search\" type=\"text\" autocomplete=\"off\"> \
             \
        </div> \
        <div class=\"col-md-4 col-sm-4 col-xs-12 padding-right0 padding-left7 xs-margin-top10 xs-padding-left0\"> \
            <button title=\"Search\" class=\"btn-general btn-blue\" id=\"submit-btn\">Search</button> \
            <a title=\"Advanced search\" class=\"btn-general btn-blue margin-left5\" href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/search/advanced-search\">Advanced search</a> \
        </div> \
    </div> \
    <div class=\"col-md-12 col-sm-12 col-xs-12 padding-left0 margin-top10 \"> \
         \
 \
        <a title=\"Browse by subject\" class=\"btn-general btn-blue margin-top5\" href=\"#\" id=\"browseBySection\">Browse by subject</a> \
    </div> \
    <div class=\"browse-by-section col-md-12 col-sm-12 col-xs-12 padding-left0 margin-top10 padding-right0\" style=\"display: none;\">         \
        <div class=\"col-sm-6 col-md-6 col-xs-10 padding-left0 padding-right0\"> \
            <select class=\"form-control browse-select-box\" id=\"subject\" placeholder=\"Select subject\"> \
                <option value=\"\" selected=\"selected\">Select subject</option> \
            <option value=\"hzc000000010000\">Fires and explosions</option><option value=\"hzc000000020000\">Waste management</option><option value=\"hzc000000030000\">Storage and transportation</option><option value=\"hzc000000040000\">Leaks and spills</option><option value=\"hzc000000050000\">Animal and microbiological hazards</option><option value=\"hzc000000060000\">Carcinogens and mutagens</option><option value=\"hzc000000070000\">Reproductive hazards</option><option value=\"hzc000000080000\">Allergens and irritants</option><option value=\"hzc000000090000\">Biological hazards</option><option value=\"hzc000000100000\">Industrial hazards</option><option value=\"hzc000000110000\">Legislation</option><option value=\"hzc000000120000\">Precautions and safe practice</option><option value=\"hzc000000130000\">Occupational health, hygiene and monitoring</option></select> \
        </div> \
        <div class=\"col-md-2 col-sm-3 col-xs-3 padding-right0 padding-left7 xs-margin-top10 xs-padding-left0\"> \
            <button title=\"Browse\" class=\"btn-general btn-blue\" id=\"browse-btn\">Browse</button> \
        </div> \
    </div> \
</div> \
 \
 \
            </div> \
 \
            <div class=\"col-md-3 col-sm-12 col-xs-12 no-padding no-padding-left-sm padding-top10-sm padding-top10-sx\"> \
                <div class=\"search-box-div height-sm\"> \
                    <div class=\"full-size\"> \
                        <div class=\"margin-left-none\"> \
                            <h3 class=\"search-heading\">Get started</h3> \
                            <ul id=\"home_rigtinfo_block\" class=\"no-padding margin-top10 margin-left10\"> \
                                <li class=\"pull-left-sm\"><a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/about\" title=\"About Laboratory Hazards Bulletin\">About Laboratory Hazards Bulletin</a></li> \
                                <li class=\"pull-left-sm\"><a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/user-guide\" title=\"User guide\">User guide</a></li> \
                                <li class=\"pull-left-sm\"><a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/about#contact-us\" title=\"Contact us\">Contact us</a></li> \
                            </ul> \
                        </div> \
                    </div> \
                </div> \
            </div> \
        </div> \
    </div> \
    <div class=\"row container-wrap row-pad-10 padding-bottom10\"> \
        <div class=\"col-md-9 pad-left0 sample-right-pad0\"> \
 \
            <div class=\"row mainresult-container margin-top20\"> \
                <div class=\"col-md-12 col-sm-12 col-xs-12\"> \
                    <h2 class=\"sample-header\">Latest articles</h2> \
                </div> \
                <div id=\"article-list\"> \
    <div class=\"col-md-12 col-sm-12 col-xs-12 xs-original-padding margin-top10\"> \
        <div class=\"media sample-box\"> \
            <div class=\"media-body\"> \
                <div> \
                    <p class=\"sample-title\"> \
                        <a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/article/J115637\" title=\"Biomonitoring of benzene and effect of wearing respirators during an Oil Spill Field Trial at Sea.\">Biomonitoring of benzene and effect of wearing respirators during an Oil Spill Field Trial at Sea.</a> \
                    </p> \
                    <hr class=\"border-bottom\"> \
                        <div class=\"sample-author\">I. Gjesteland, B. E. Hollund, J. Kirkeleit, P. Daling and M. Bratveit</div> \
 \
                    <div class=\"sample-link\"> \
                        <div> \
                                <p class=\"margin-bottom0\"> \
                                    <strong><i>Ann. Work Exposures Health</i></strong>, \
<span>Oct 2018</span>, \
<span>62</span><span> (8)</span>, \
                                    <span>1033-1039 \
                                    </span> \
                                </p> \
                                                            <p class=\"margin-bottom0\"> \
                                    <strong>Full text: </strong><a href=\"http://dx.doi.org/10.1093/annweh/wxy067\" title=\"http://dx.doi.org/10.1093/annweh/wxy067\">http://dx.doi.org/10.1093/annweh/wxy067</a> \
                                </p> \
                                                            <p><strong>Subject category: </strong><span>Precautions and safe practice</span></p> \
                        </div> \
                    </div> \
                </div> \
            </div> \
        </div> \
    </div> \
    <div class=\"col-md-12 col-sm-12 col-xs-12 xs-original-padding margin-top10\"> \
        <div class=\"media sample-box\"> \
            <div class=\"media-body\"> \
                <div> \
                    <p class=\"sample-title\"> \
                        <a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/article/J115638\" title=\"Predictors for increased and reduced rat and mouse allergen exposure in laboratory animal facilities.\">Predictors for increased and reduced rat and mouse allergen exposure in laboratory animal facilities.</a> \
                    </p> \
                    <hr class=\"border-bottom\"> \
                        <div class=\"sample-author\">A. Straumfors, W. Eduard, K. Andresen and A. K. Sjaastad</div> \
 \
                    <div class=\"sample-link\"> \
                        <div> \
                                <p class=\"margin-bottom0\"> \
                                    <strong><i>Ann. Work Exposures Health</i></strong>, \
<span>Oct 2018</span>, \
<span>62</span><span> (8)</span>, \
                                    <span>953-965 \
                                    </span> \
                                </p> \
                                                            <p class=\"margin-bottom0\"> \
                                    <strong>Full text: </strong><a href=\"http://dx.doi.org/10.1093/annweh/wxy060\" title=\"http://dx.doi.org/10.1093/annweh/wxy060\">http://dx.doi.org/10.1093/annweh/wxy060</a> \
                                </p> \
                                                            <p><strong>Subject category: </strong><span>Allergens and irritants</span></p> \
                        </div> \
                    </div> \
                </div> \
            </div> \
        </div> \
    </div> \
    <div class=\"col-md-12 col-sm-12 col-xs-12 xs-original-padding margin-top10\"> \
        <div class=\"media sample-box\"> \
            <div class=\"media-body\"> \
                <div> \
                    <p class=\"sample-title\"> \
                        <a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/article/J115639\" title=\"Allergic contact dermatitis caused by dexpanthenol - Probably a frequent allergen.\">Allergic contact dermatitis caused by dexpanthenol - Probably a frequent allergen.</a> \
                    </p> \
                    <hr class=\"border-bottom\"> \
                        <div class=\"sample-author\">R. A. Fernandes, L. Santiago, M. Gouveia and M. Goncalo</div> \
 \
                    <div class=\"sample-link\"> \
                        <div> \
                                <p class=\"margin-bottom0\"> \
                                    <strong><i>Contact Dermatitis</i></strong>, \
<span>Nov 2018</span>, \
<span>79</span><span> (5)</span>, \
                                    <span>276-280 \
                                    </span> \
                                </p> \
                                                            <p class=\"margin-bottom0\"> \
                                    <strong>Full text: </strong><a href=\"http://dx.doi.org/10.1111/cod.13054\" title=\"http://dx.doi.org/10.1111/cod.13054\">http://dx.doi.org/10.1111/cod.13054</a> \
                                </p> \
                                                            <p><strong>Subject category: </strong><span>Allergens and irritants</span></p> \
                        </div> \
                    </div> \
                </div> \
            </div> \
        </div> \
    </div> \
    <div class=\"col-md-12 col-sm-12 col-xs-12 xs-original-padding margin-top10\"> \
        <div class=\"media sample-box\"> \
            <div class=\"media-body\"> \
                <div> \
                    <p class=\"sample-title\"> \
                        <a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/article/J115641\" title=\"Ten-year trends in contact allergy to formaldehyde and formaldehyde-releasers.\">Ten-year trends in contact allergy to formaldehyde and formaldehyde-releasers.</a> \
                    </p> \
                    <hr class=\"border-bottom\"> \
                        <div class=\"sample-author\">I. M. Fasth, N. H. Ulrich and J. D. Johansen</div> \
 \
                    <div class=\"sample-link\"> \
                        <div> \
                                <p class=\"margin-bottom0\"> \
                                    <strong><i>Contact Dermatitis</i></strong>, \
<span>Nov 2018</span>, \
<span>79</span><span> (5)</span>, \
                                    <span>263-269 \
                                    </span> \
                                </p> \
                                                            <p class=\"margin-bottom0\"> \
                                    <strong>Full text: </strong><a href=\"http://dx.doi.org/10.1111/cod.13052\" title=\"http://dx.doi.org/10.1111/cod.13052\">http://dx.doi.org/10.1111/cod.13052</a> \
                                </p> \
                                                            <p><strong>Subject category: </strong><span>Allergens and irritants</span></p> \
                        </div> \
                    </div> \
                </div> \
            </div> \
        </div> \
    </div> \
    <div class=\"col-md-12 col-sm-12 col-xs-12 xs-original-padding margin-top10\"> \
        <div class=\"media sample-box\"> \
            <div class=\"media-body\"> \
                <div> \
                    <p class=\"sample-title\"> \
                        <a href=\"http://pubs.rsc.org/lus/laboratory-hazards-bulletin/article/J115646\" title=\"A suspect screening method for characterizing multiple chemical exposures among a demographically diverse population of pregnant women in San Francisco.\">A \
 suspect screening method for characterizing multiple chemical exposures \
 among a demographically diverse population of pregnant women in San  \
Francisco.</a> \
                    </p> \
                    <hr class=\"border-bottom\"> \
                        <div class=\"sample-author\">A. Wang, R. R. Gerona, J. M. Schwartz, T. Lin, M. Sirota, R. Morello-Frosch and T. J. Woodruff</div> \
 \
                    <div class=\"sample-link\"> \
                        <div> \
                                <p class=\"margin-bottom0\"> \
                                    <strong><i>Environ. Health Perspect.</i></strong>, \
<span>Jul 2018</span>, \
<span>126</span><span> (7)</span>, \
                                    <span>077009 \
                                    </span> \
                                </p> \
                                                            <p class=\"margin-bottom0\"> \
                                    <strong>Full text: </strong><a href=\"http://dx.doi.org/10.1289/EHP2920\" title=\"http://dx.doi.org/10.1289/EHP2920\">http://dx.doi.org/10.1289/EHP2920</a> \
                                </p> \
                                                            <p><strong>Subject category: </strong><span>Occupational health, hygiene and monitoring</span></p> \
                        </div> \
                    </div> \
                </div> \
            </div> \
        </div> \
    </div> \
<script type=\"text/javascript\"> \
    var pagenumber = \"1\"; \
    var pageSize = \"5\";     \
    var totalCount = \"75766\"; \
    var count = 0; \
    var isAllLoaded = false; \
 \
    $(document).ready(function(e) {        \
 \
        if (totalCount > (pageSize * pagenumber)) { \
            $(\"#show-more\").show(); \
        } else { \
            $(\"#show-more\").hide(); \
        } \
    }); \
 \
</script> \
                </div> \
            </div> \
 \
            <div class=\"row margin-top10\"> \
                <div id=\"lazyloading\" class=\"fl\" style=\"display: none;\"> \
                    <img alt=\"Loading\" class=\"lazyloading\" style=\"margin-left: 345px; margin-right: 348px;\" src=\"Laboratory%20Hazards%20Bulletin_files/ajax-loader.gif\"> \
                </div> \
                <div class=\"col-md-12 padding-right10 xs-padding-right15 margin-top10\" id=\"show-more\"> \
                    <div class=\"alert-show-more\" title=\"Show more\"> \
                        Show more \
                    </div> \
                </div> \
            </div> \
        </div> \
        <div class=\"col-md-3 panel-box\"> \
 \
            <div class=\"row product-box-bg\"> \
                <div class=\"col-sm-6 col-md-12 col-xs-12 panel-box-border\"> \
 \
                    <div class=\"panel panel-default panel-info-blocks\"> \
                        <div class=\"panel-heading panel-info-blocks-heading\"> \
                            <h3 class=\"clearfix panel-title\"> \
                                <span class=\"pull-left col-xs-9 col-sm-9 col-md-9 col-lg-9  panel-info-container-title\">Related products \
                                </span> \
                            </h3> \
                        </div> \
                        <div class=\"margin-top10\"> \
                            <div id=\"listOfRelatedProduct\">    <div class=\"carousel relatedProducts\"> \
        <div class=\"slides\" style=\"width: 240px; height: 170px;\"> \
                <div class=\"compound slideItem\" style=\"width: 160px; height: 172px; top: 20px; right: 40px; opacity: 1; z-index: 3;\" id=\"current\"> \
                    <a href=\"http://www.rsc.org/merck-index\" title=\"The Merck Index* Online\"> \
                        <img alt=\"The Merck Index* Online\" src=\"Laboratory%20Hazards%20Bulletin_files/merck-index.png\" style=\"width: 158px; height: 120px;\"> \
                        <span class=\"compound_desc\" title=\"The Merck Index* Online\">The Merck Index* <i>Online</i></span> \
                    </a> \
                <div class=\"shadow\" style=\"width: 158px; z-index: -1; position: absolute; margin: 0px; padding: 0px; overflow: hidden; left: 0px; bottom: 0px;\"><div class=\"shadowLeft\" style=\"position: relative; float: left;\"></div><div class=\"shadowMiddle\" style=\"position: relative; float: left; width: 158px;\"></div><div class=\"shadowRight\" style=\"position: relative; float: left;\"></div></div></div> \
                <div class=\"compound slideItem\" style=\"width: 126.4px; height: 125.6px; top: 32px; right: 21.34px; opacity: 1; z-index: 2;\"> \
                    <a href=\"http://pubs.rsc.org/lus/chemical-hazards-industry\" title=\"Chemical Hazards in Industry\"> \
                        <img alt=\"Chemical Hazards in Industry\" src=\"Laboratory%20Hazards%20Bulletin_files/chi.png\" style=\"width: 124.4px; height: 96px;\"> \
                        <span class=\"compound_desc\" title=\"Chemical Hazards in Industry\">Chemical Hazards in Industry</span> \
                    </a> \
                <div class=\"shadow\" style=\"width: 158px; z-index: -1; position: absolute; margin: 0px; padding: 0px; overflow: hidden; left: 0px; bottom: 0px;\"><div class=\"shadowLeft\" style=\"position: relative; float: left;\"></div><div class=\"shadowMiddle\" style=\"position: relative; float: left; width: 124.4px;\"></div><div class=\"shadowRight\" style=\"position: relative; float: left;\"></div></div></div> \
                <div class=\"compound slideItem\" style=\"width: 99.52px; height: 104.48px; top: 41.6px; right: 70.24px; opacity: 0; z-index: 1; display: none;\"> \
                    <a href=\"http://pubs.rsc.org/en/journals/journalissues/em\" title=\"Environmental Science: Processes &amp; Impacts\"> \
                        <img alt=\"Environmental Science: Processes &amp; Impacts\" src=\"Laboratory%20Hazards%20Bulletin_files/ESPI.png\" style=\"width: 97.52px; height: 76.8px;\"> \
                        <span class=\"compound_desc\" title=\"Environmental Science: Processes &amp; Impacts\">Environmental Science: Processes &amp; Impacts</span> \
                    </a> \
                <div class=\"shadow\" style=\"width: 158px; z-index: -1; position: absolute; margin: 0px; padding: 0px; overflow: hidden; left: 0px; bottom: 0px;\"><div class=\"shadowLeft\" style=\"position: relative; float: left;\"></div><div class=\"shadowMiddle\" style=\"position: relative; float: left; width: 97.52px;\"></div><div class=\"shadowRight\" style=\"position: relative; float: left;\"></div></div></div> \
                <div class=\"compound slideItem\" style=\"width: 126.4px; height: 145.6px; top: 32px; right: 92.26px; opacity: 1; z-index: 2;\"> \
                    <a href=\"http://pubs.rsc.org/en/journals/journalissues/re\" title=\"Reaction Chemistry &amp; Engineering\"> \
                        <img alt=\"Reaction Chemistry &amp; Engineering\" src=\"Laboratory%20Hazards%20Bulletin_files/RCE.png\" style=\"width: 122.4px; height: 96px;\"> \
                        <span class=\"compound_desc\" title=\"Reaction Chemistry &amp; Engineering\">Reaction Chemistry &amp; Engineering</span> \
                    </a> \
                <div class=\"shadow\" style=\"width: 158px; z-index: -1; position: absolute; margin: 0px; padding: 0px; overflow: hidden; left: 0px; bottom: 0px;\"><div class=\"shadowLeft\" style=\"position: relative; float: left;\"></div><div class=\"shadowMiddle\" style=\"position: relative; float: left; width: 122.4px;\"></div><div class=\"shadowRight\" style=\"position: relative; float: left;\"></div></div></div> \
        <div class=\"spinner\" style=\"display: none; width: 158px; height: 120px; top: 20px; right: 40px; opacity: 1; z-index: 7; position: absolute; cursor: pointer; overflow: hidden; padding: 0px; margin: 0px;\"></div><div class=\"videoOverlay\" style=\"display: none; width: 158px; height: 120px; top: 20px; right: 40px; opacity: 1; z-index: 6; position: absolute; cursor: pointer; overflow: hidden; padding: 0px; margin: 0px; border: medium none;\"></div></div> \
    <div class=\"nextButton\" title=\"Next\"></div><div class=\"prevButton\" title=\"Previous\"></div><div class=\"icon_video\"></div></div> \
     \
</div> \
                        </div> \
                    </div> \
 \
                </div> \
            </div> \
            <div class=\"row product-box-bg\"> \
 \
 \
<div class=\"panel panel-default panel-info-blocks\"> \
    <div class=\"panel-heading panel-info-blocks-heading\"> \
        <h3 class=\"clearfix panel-title\"> \
            <span class=\"pull-left col-xs-9 col-sm-9 col-md-9 col-lg-9  panel-info-container-title\">Meet the team \
            </span> \
        </h3> \
 \
    </div> \
    <div class=\"pull-left margin-top10 padding-left10 padding-right10\"> \
            <div class=\"margin-bottom10\"> \
                <p class=\"result-text-bold\">Kirsty Muirhead</p> \
                <p>Data Content Editor</p> \
            </div> \
            <div class=\"margin-bottom10\"> \
                <p class=\"result-text-bold\">Sayeeda Qureshi</p> \
                <p>Publishing Assistant – Databases</p> \
            </div> \
            <div class=\"margin-bottom10\"> \
                <p class=\"result-text-bold\">Fiona Tscherny</p> \
                <p>Publishing Assistant – Databases</p> \
            </div> \
        <div class=\"pull-left padding-bottom10\">             \
Please feel free to contact the Laboratory Hazards Bulletin team at <a title=\"rscindex@rsc.org\" href=\"mailto:rscindex@rsc.org\"><u>rscindex@rsc.org</u></a> with any questions or comments. \
        </div> \
    </div> \
</div> \
 \
            </div> \
        </div> \
    </div> \
</div> \
 \
 \
    <div class=\"container hide\"> \
        <div class=\"content-block clearfix padding-bottom10\"> \
            <div class=\"col-sm-12 col-md-12 col-xs-12 \"> \
                © Royal Society of Chemistry \
            </div> \
        </div> \
    </div> \
    <div class=\"clearfix container padding-left0 margin-top10 padding-right0\"> \
        <div id=\"footer\"> \
 \
<link href=\"Laboratory%20Hazards%20Bulletin_files/museo-sans-500.css\" rel=\"Stylesheet\" type=\"text/css\"> \
 \
 \
<!--tabindex minimum value is 10001 as below that value, \
    all tabindex values are reserved for the consumer application like learn chemestry,ecommerce -> \
 \
<!--------------------------------------> \
<link href=\"Laboratory%20Hazards%20Bulletin_files/RSCHeaderFooterNew.css\" rel=\"stylesheet\" type=\"text/css\" media=\"interactive, braille, emboss, handheld, projection, screen, tty, tv\"> \
<!--[if lte IE 7]> \
    <style> \
	div.footer_address_rsc_001, div.footer_menus_rsc_001, ul.f_menu_rsc_001 { \
	zoom: 1; \
	display:inline; \
	} \
    a.fm_link_rsc_001{margin-left:-15px;*margin-left:0px;} \
	</style> \
	<![endif]--> \
<!--[if IE 6]>    \
    <style type=\"text/css\"> \
	.footer_rsc_001{padding-bottom:0px;} \
   	</style>	 \
   <![endif]--> \
<div class=\"pagewidth\" id=\"rsc-global-footer\"> \
    <div id=\"rsc-global-footer-inner\"> \
        <div id=\"rsc-global-footer-inner-div\"> \
             \
            <ul id=\"rsc-global-footer-link-container\"> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/aboutus/\" title=\"About us\" target=\"_self\"> \
                    About us</a> </li> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/AboutUs/rscwork/index.asp\" title=\"Working for us\" target=\"_self\"> \
                    Working for us</a> </li> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/AboutUs/News/PressReleases/index.asp\" title=\"Press office\" target=\"_self\"> \
                    Press office</a> </li> \
                 \
            </ul> \
             \
            <ul id=\"rsc-global-footer-link-container\"> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/help/termsconditions.asp\" title=\"Terms of use\" target=\"_self\"> \
                    Terms of use</a> </li> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/help/privacy.asp\" title=\"Privacy &amp; cookies\" target=\"_self\"> \
                    Privacy &amp; cookies</a> </li> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/help/accessibility.asp\" title=\"Accessibility\" target=\"_self\"> \
                    Accessibility</a> </li> \
                 \
            </ul> \
             \
            <ul id=\"rsc-global-footer-link-container\"> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/help/\" title=\"Help\" target=\"_self\"> \
                    Help</a> </li> \
                 \
                <li><a class=\"fm_link_rsc_001\" href=\"http://www.rsc.org/AboutUs/Contacts/index.asp\" title=\"Contact\" target=\"_self\"> \
                    Contact</a> </li> \
                 \
            </ul> \
             \
        </div> \
        <span class=\"copyright\">© Royal Society of Chemistry 2020<br> \
            Registered charity number: 207890</span> \
    </div> \
</div> \
 \
        </div> \
    </div> \
 \
</body></html>", ""},
    {"buscador", 0, 1, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"> \
<!-- FILE NAME: /exlibris/metalib/m4_1/inovo/www_v_por/find-db-2-head --> \
<html lang=\"en\"><head>  \
 \
<!-- START INCLUDE FILE NAME:/exlibris/metalib/m4_1/inovo/www_v_por/meta-tags --> \
  \
<meta http-equiv=\"Cache-Control\" content=\"no-cache\">  \
<meta http-equiv=\"Pragma\" content=\"no-cache\">  \
<meta http-equiv=\"Expires\" content=\"Tue, 20 Aug 1996 14:25:27 GMT\">  \
<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">  \
<!-- meta http-equiv=\"refresh\" content=\"9999; url=https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00001?func=exit\" -->  \
<script src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/include-set-cookie.js\" type=\"text/javascript\"></script><style>@media print {#ghostery-purple-box {display:none !important}}</style>  \
  \
<link href=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/principal.css\" type=\"text/css\" rel=\"stylesheet\">  \
<link href=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/mapa-site.css\" type=\"text/css\" rel=\"stylesheet\">  \
<link href=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/tags.css\" type=\"text/css\" rel=\"stylesheet\">  \
<link href=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/tags.css\" type=\"text/css\" rel=\"stylesheet\">  \
<link href=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/bootstrap.css\" type=\"text/css\" rel=\"stylesheet\">  \
<link href=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/template-azul.css\" type=\"text/css\" rel=\"stylesheet\">  \
<link href=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/font-awesome.css\" type=\"text/css\" rel=\"stylesheet\">  \
  \
<!--[if IE 7]>  \
<link href=\"https://www.example.com/templates/periodicos/css/ie7.css\" type=\"text/css\" rel=\"stylesheet\" /><![endif]-->  \
  \
<style>  \
label {display:inline !important;}  \
</style>  \
 \
<!-- END OF INCLUDE FILE:/exlibris/metalib/m4_1/inovo/www_v_por/meta-tags --> \
  \
<title>MetaLib® - Buscar Base</title>  \
<script src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/include-window.js\" type=\"text/javascript\"></script>  \
<script src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/scripts.js\" type=\"text/javascript\"></script>  \
<script src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/find-db-2.js\" type=\"text/javascript\"></script>  \
</head>  \
  \
<body onload='javascript:setWindowName();SetImages(\"https://buscador.example.com/INOVO/icon_por\",\"Adicionado ao Meu Espaço > Minhas Bases\"); setCookie(\"74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T\",\"buscador.example.com\");resizeIfrm();' style=\"background:none;background-color:#FFFFFF;background-image:none;\">  \
 \
<!-- START INCLUDE FILE NAME:/exlibris/metalib/m4_1/inovo/www_v_por/banner --> \
  \
 \
<!-- END OF INCLUDE FILE:/exlibris/metalib/m4_1/inovo/www_v_por/banner --> \
  \
<!--  \
<a class=\"skiplink\" href=\"#startcontent\" title=\"Pular a  navegação\">Pular a  navegação</a>  \
-->  \
<div id=\"headerwrap\">  \
<!-- FILE NAME: /exlibris/metalib/m4_1/inovo/www_v_por/navigation-guest --> \
<script language=\"javascript\">  \
  \
loc = location.href;  \
get = strstr(loc, '?');  \
mode = strstr(get, 'type=');  \
//document.write(loc);  \
autenticado = strstr(get, 'autenticado=');  \
func = loc.slice(loc.indexOf(\"func=\")+5);  \
func = func.slice(0, func.indexOf(\"&\"));  \
//autenticado = strstr(autenticado, '&', true);  \
if(autenticado != \"\"){  \
        autenticado = autenticado.substr(12,1);  \
        //document.write(autenticado);  \
} else {  \
        autenticado = false;  \
}  \
  \
if(mode.length > 6){  \
        mode = strstr(mode, '&', true);  \
}  \
if(mode != false){  \
        mode = mode.substr(5);  \
} else {  \
        mode = strstr(get, 'typep=');  \
        if(mode != false && mode != \"\"){  \
                mode = mode.substr(5);  \
        }  \
}  \
  \
function strstr (haystack, needle, bool) {  \
                    // http://kevin.vanzonneveld.net  \
                    // +   original by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)  \
                    // +   bugfixed by: Onno Marsman  \
                    // +   improved by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)  \
                    // *     example 1: strstr('Kevin van Zonneveld', 'van');  \
                    // *     returns 1: 'van Zonneveld'  \
                    // *     example 2: strstr('Kevin van Zonneveld', 'van', true);  \
                    // *     returns 2: 'Kevin '  \
                    // *     example 3: strstr('name@example.com', '@');  \
                    // *     returns 3: '@example.com'  \
                    // *     example 4: strstr('name@example.com', '@', true);  \
                    // *     returns 4: 'name'  \
  \
                    var pos = 0;  \
  \
                    haystack += '';  \
                    pos = haystack.indexOf( needle );  \
                    if (pos == -1) {  \
                        return false;  \
                    } else{  \
                        if (bool){  \
                            return haystack.substr( 0, pos );  \
                        } else{  \
                            return haystack.slice( pos );  \
                        }  \
                    }  \
                }  \
  \
  \
  \
                var elapsedtime = 0;  \
                var div = '';  \
                var lnk = '';  \
                var stop = false;  \
  \
                function timer(){  \
                        if (elapsedtime >= 3000){  \
                                resetMn();  \
                        } else{  \
                                if (!stop){  \
                                        elapsedtime += 10;  \
                                }  \
                                setTimeout('timer()', 10);  \
                        }  \
                }  \
  \
                function resetMn(){  \
                        mnuHideAll();  \
                        lnkHideAll();  \
                        if (div != ''){  \
                                mnuShow(div);  \
                        }  \
                        if (lnk != ''){  \
                                lnkShow(lnk);  \
                        }  \
                }  \
  \
                function lnkShow(div){  \
                        lnkHideAll();  \
                        document.getElementById(div).className = 'itenAtivo';  \
                }  \
  \
                function lnkHideAll(){  \
                        document.getElementById('lnkMn225').className = '';document.getElementById('lnkMn226').className = '';          }  \
  \
                function mnuShow(div){  \
                        mnuHideAll();  \
                        document.getElementById(div).style.display = 'block';  \
                        elapsedtime = 0;  \
                        //timer();  \
                }  \
  \
                function setClassLinks(div){  \
                        tagsa = document.getElementById(div).getElementsByTagName('a');  \
                        for(var i = 0; i < tagsa.length; i++) {  \
                                tagsa[i].setAttribute('class', 'selecionado');  \
                        }  \
                }  \
  \
                function mnuHideAll(){  \
                        document.getElementById('divMn225').style.display = 'none';document.getElementById('divMn226').style.display = 'none';document.getElementById('divMn227').style.display = 'none';document.getElementById('divMn228').style.display = 'none'; }  \
  \
  \
                //var timeout = 0;  \
  \
                //function resetTimeout(){  \
                        //timeout = 0;  \
                //}  \
  \
                function getNovaBusca() {  \
                                        var ca = document.cookie.split(';');  \
                                        var tipoBusca = '';  \
  \
  \
                                        for(var i=0;i < ca.length;i++) {  \
                                                var c = ca[i];  \
                                                while (c.charAt(0)==' ') c = c.substring(1,c.length);  \
                                                if (c.indexOf('busca=') == 0){  \
                                                        tipoBusca = c.substring(6,c.length);  \
                                                        break;  \
                                                }  \
                                        }  \
  \
                        if(tipoBusca == '') {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00002?func=meta-1&type=m&i=3&mode1=simple';  \
                        } else if(tipoBusca == 'metabuscasimples') {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00003?func=meta-1&type=m&i=3&mode1=simple';  \
                        } else if(tipoBusca == 'metabuscaavancada') {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00004?func=meta-1&type=m&i=3&mode=simple&mode1=advanced';  \
                        } else {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00005?func=meta-1&type=m&i=3&mode1=simple';  \
                        }  \
  \
                }  \
  \
                function novaBuscaPeriodicos() {  \
                                        var ca = document.cookie.split(';');  \
                                        var tipoBusca = '';  \
  \
  \
                                        for(var i=0;i < ca.length;i++) {  \
                                                var c = ca[i];  \
                                                while (c.charAt(0)==' ') c = c.substring(1,c.length);  \
                                                if (c.indexOf('busca=') == 0){  \
                                                        tipoBusca = c.substring(6,c.length);  \
                                                        break;  \
                                                }  \
                                        }  \
  \
                        if(tipoBusca == '') {  \
                                document.getElementById('find-ej-body-frame').src = 'https://link.example.com/sfxlcl41/az/grupo02?&param_sid_save=cb3984dc768b42d830e684db36eabd88&param_lang_save=por&param_embedded_save=ML&param_letter_group_save=&param_perform_save=locate&param_letter_group_script_save=&param_chinese_checkbox_save=0&param_services2filter_save=getFullTxt&param_current_view_save=table&param_jumpToPage_save=1&param_type_save=textSearch&param_textSearchType_save=contains&&param_perform_value=locate';  \
                        } else if(tipoBusca == 'periodicosavancada') {  \
                                document.getElementById('find-ej-body-frame').src = 'https://link.example.com/sfxlcl41/az/grupo02?&param_sid_save=cb3984dc768b42d830e684db36eabd88&param_lang_save=por&param_embedded_save=ML&param_letter_group_save=&param_perform_save=locate&param_letter_group_script_save=&param_chinese_checkbox_save=0&param_services2filter_save=getFullTxt&param_current_view_save=table&param_jumpToPage_save=1&param_type_save=textSearch&param_textSearchType_save=contains&&param_perform_value=locate';  \
                        } else if(tipoBusca == 'periodicosareaconhecimento') {  \
                                document.getElementById('find-ej-body-frame').src = 'https://link.example.com/sfxlcl41/az/grupo02?&param_sid_save=cb3984dc768b42d830e684db36eabd88&param_lang_save=por&param_embedded_save=ML&param_letter_group_save=&param_perform_save=locate&param_letter_group_script_save=&param_chinese_checkbox_save=0&param_services2filter_save=getFullTxt&param_current_view_save=table&param_jumpToPage_save=1&param_type_save=textSearch&param_textSearchType_save=contains&&param_perform_value=searchCategories';  \
                        } else if(tipoBusca == 'periodicosreferencia') {  \
                                document.getElementById('find-ej-body-frame').src = 'https://link.example.com/sfxlcl41/az/grupo02?&param_sid_save=cb3984dc768b42d830e684db36eabd88&param_lang_save=por&param_embedded_save=ML&param_letter_group_save=&param_perform_save=locate&param_letter_group_script_save=&param_chinese_checkbox_save=0&param_services2filter_save=getFullTxt&param_current_view_save=table&param_jumpToPage_save=1&param_type_save=textSearch&param_textSearchType_save=contains&&param_perform_value=citation';  \
                        } else {  \
                                if(document.getElementById('find-ej-body-frame') != null){  \
                                        document.getElementById('find-ej-body-frame').src = 'https://link.example.com/sfxlcl41/az/grupo02?&param_sid_save=cb3984dc768b42d830e684db36eabd88&param_lang_save=por&param_embedded_save=ML&param_letter_group_save=&param_perform_save=locate&param_letter_group_script_save=&param_chinese_checkbox_save=0&param_services2filter_save=getFullTxt&param_current_view_save=table&param_jumpToPage_save=1&param_type_save=textSearch&param_textSearchType_save=contains&&param_perform_value=locate';  \
                                }else{  \
                                        document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00006?func=find-ej-1&institute=CAPES&portal=CAPES&new_lng=POR&type=p&sfxgroup=&adv=1&param_sid_save=914bf749a4b578a3442a5ef3dec62f1d&param_embedded_save=ML&param_lang_save=por&param_letter_group_save=&param_perform_save=locate&param_letter_group_script_save=&param_chinese_checkbox_save=0&param_services2filter_save=getFullTxt&param_current_view_save=table&param_jumpToPage_save=1&param_type_save=textSearch&param_textSearchType_save=contains'  \
                                }  \
                        }  \
                }  \
  \
                function novaBuscaBase() {  \
                                        var ca = document.cookie.split(';');  \
                                        var tipoBusca = '';  \
  \
  \
                                        for(var i=0;i < ca.length;i++) {  \
                                                var c = ca[i];  \
                                                while (c.charAt(0)==' ') c = c.substring(1,c.length);  \
                                                if (c.indexOf('busca=') == 0){  \
                                                        tipoBusca = c.substring(6,c.length);  \
                                                        break;  \
                                                }  \
                                        }  \
  \
                        if(tipoBusca == '') {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00007?func=find-db-1&type=b&i=0';  \
                        } else if(tipoBusca == 'baseportitulo') {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00008?func=find-db-1&type=b&i=0';  \
                        } else if(tipoBusca == 'baseporareaconhecimento') {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00009?func=find-db-1&mode=category&type=b';  \
                        } else if(tipoBusca == 'baseavancada') {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00010?func=find-db-1&mode=locate&type=b';  \
                        } else {  \
                                document.location.href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00011?func=find-db-1&type=b&i=0';  \
                        }  \
                }  \
  \
        </script>  \
  \
  \
        <iframe id=\"ifrmResize\" src=\"https://www.example.com/?height=1214&amp;option=com_pmetabusca&amp;view=pmetabuscaresize&amp;tmpl=component\" width=\"0\" height=\"0\" frameborder=\"0\"></iframe>  \
  \
        <div id=\"iMnuMetaBusca\" style=\"diplay:none\">  \
        </div>  \
        <div id=\"iMnuMetaResultado\" style=\"diplay:none\">  \
        </div>  \
        <div id=\"iMnuMetaAnteriores\" style=\"diplay:none\">  \
        </div>  \
        <div id=\"iMnuMetaRefinar\" style=\"diplay:none\">  \
        </div>  \
  \
  \
        <div class=\"homeConteudoPrincipal\" id=\"metalibNavMenu\" style=\"width: 95%;\">  \
  \
        <div id=\"MnuMetaBusca\" style=\"display:none\">  \
                <div class=\"boxAcervo2\">  \
                        <span class=\"titulo2\" style=\"*heigth:10px;margin-left:5px; display: none;\">  \
                                <a id=\"aMnuMetaBusca\" style=\"float:left; display: none;\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"javascript:getNovaBusca();\">Nova Busca</a>  \
                                <div id=\"MnuMetaBuscaFull\" style=\"float:left;display:none\">  \
  \
                                        <script>  \
                                        //if(true == true){  \
                                                //document.write('<div style=\"float:left\" id=\"aMnuMetaRefinar\" style=\"display:block\">');  \
                                                //document.write('&nbsp; | &nbsp;');  \
                                                //document.write('<a id=\"aMnuMetaBusca1\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00012?func=meta-3&type=m&i=2\">Resultado da Busca</a>');  \
                                                document.write('&nbsp; | &nbsp;');  \
                                                document.write('<a id=\"aMnuMetaBusca2\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00013?func=meta-4&type=m&i=1\">Buscas Anteriores</a>');  \
                                                document.write('&nbsp; | &nbsp;');  \
                                                document.write('<a id=\"aMnuMetaBusca3\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00014?func=meta-5&type=m&i=0\">Refinar Busca</a>');  \
                                                //document.write('</div>');  \
                                        //}  \
                                        </script>&nbsp; | &nbsp;<a id=\"aMnuMetaBusca2\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00013?func=meta-4&amp;type=m&amp;i=1\">Buscas Anteriores</a>&nbsp; | &nbsp;<a id=\"aMnuMetaBusca3\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00014?func=meta-5&amp;type=m&amp;i=0\">Refinar Busca</a>  \
  \
                                </div>  \
                        </span>  \
                        <span class=\"\" style=\"*heigth:10px;margin-left:1px;float:right;\">  \
                                <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00015?func=help&amp;file_name=FIND-DB&amp;section=list%27\" title=\"Ajuda\" accesskey=\"h\" onclick=\"javascript:open_window_help('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00016?func=help&amp;file_name=FIND-DB&amp;section=list');return false;\"><span class=\"ActionText\">Ajuda</span></a>  \
                        </span>  \
  \
                </div>  \
        </div>  \
        <div id=\"MnuMetaBase\" style=\"display: block;\">  \
                <div class=\"boxAcervo2\">  \
                        <span class=\"titulo2\" style=\"*heigth:10px;margin-left:5px; display: none;\">  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"javascript:novaBuscaBase();\">Nova Busca</a>  \
                                <!--  \
                                &nbsp; | &nbsp;  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00017?func=find-db-0&type=b&i=1\">Resultado da Busca</a>  \
                                -->  \
                        </span>  \
                        <span class=\"\" style=\"*heigth:10px;margin-left:1px;float:right;\">  \
                                <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00018?func=help&amp;file_name=FIND-DB&amp;section=list%27\" title=\"Ajuda\" accesskey=\"h\" onclick=\"javascript:open_window_help('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00019?func=help&amp;file_name=FIND-DB&amp;section=list');return false;\"><span class=\"ActionText\">Ajuda</span></a>  \
                        </span>  \
                </div>  \
        </div>  \
        <div id=\"MnuMetaPeriodicos\" style=\"display:none\">  \
                <div class=\"boxAcervo2\">  \
                        <span class=\"titulo2\" style=\"*heigth:10px;margin-left:5px; display: none;\">  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"javascript:novaBuscaPeriodicos();\">Nova Busca</a>  \
                        </span>  \
                        <span class=\"\" style=\"*heigth:10px;margin-left:1px;float:right;\">  \
                                <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00020?func=help&amp;file_name=FIND-DB&amp;section=list%27\" title=\"Ajuda\" accesskey=\"h\" onclick=\"javascript:open_window_help('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00021?func=help&amp;file_name=FIND-DB&amp;section=list');return false;\"><span class=\"ActionText\">Ajuda</span></a>  \
                        </span>  \
                </div>  \
        </div>  \
        <div id=\"MnuMetaMeuEspaco\" style=\"display:none\">  \
                <div class=\"boxAcervo2\">  \
                        <span class=\"titulo2\" style=\"*heigth:10px;margin-left:5px;\">  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00022?func=eshelf-2&amp;type=e&amp;i=3\">Meus Artigos</a>  \
                                &nbsp; | &nbsp;  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00023?func=my-sets-1&amp;type=e&amp;i=2\">Meus Conjuntos de Bases</a>  \
                                &nbsp; | &nbsp;  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00024?func=my-ejournal-1&amp;type=e&amp;i=1\">Meus Periodicos</a>  \
                                &nbsp; | &nbsp;  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00025?func=history-1&amp;type=e&amp;i=0\">Minhas Estrategias de Busca</a>  \
                                <script type=\"text/javascript\" src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/a_002.html\"></script>  \
                                &nbsp; | &nbsp;  \
                                <a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://www.example.com/?option=com_ptreinaments&amp;view=ptreinamentsspace\" target=\"_parent\">Meus Certificados</a>  \
                        </span>  \
                        <span class=\"\" style=\"*heigth:10px;margin-left:1px;float:right;\">  \
                                <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00026?func=help&amp;file_name=FIND-DB&amp;section=list%27\" title=\"Ajuda\" accesskey=\"h\" onclick=\"javascript:open_window_help('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00027?func=help&amp;file_name=FIND-DB&amp;section=list');return false;\"><span class=\"ActionText\">Ajuda</span></a>  \
                        </span>  \
                </div>  \
        </div>  \
  \
        <script language=\"javascript\">  \
                window.onresize = function(){  \
                        resizeIfrm();  \
                }  \
                function resizeIfrm() {  \
                        var iframe = document.getElementById('ifrmResize');  \
                var height = Math.max(document.body.offsetHeight, document.body.scrollHeight);  \
                //var height = Math.max(3000);  \
                //var height += 50;  \
                iframe.src = 'https://www.example.com/?height='+height + '&option=com_pmetabusca&view=pmetabuscaresize&tmpl=component';  \
                //alert(mode);  \
                //if(mode == false || mode == 'm'){  \
                //      document.getElementById('MnuMetaBusca').style.display = 'block';  \
                //}  \
                }  \
                //setTimeout('sessionTimeout()', (30 * 60 * 1000));     // 30 minutes  \
                //function sessionTimeout(){  \
                //      alert('Acesso expirou por inatividade. Por favor, identifique-se novamente.');  \
                //      window.top.location.href = 'http://buscador.example.com:80/V/?func=exit';  \
                //      window.location.reload();  \
                //}  \
        </script>  \
<script>  \
        if(mode == 'p' || (func.indexOf(\"find-ej-1\") != -1) ){  \
                document.getElementById('MnuMetaPeriodicos').style.display = 'block';  \
        } else if(mode == 'm'){  \
                document.getElementById('MnuMetaBusca').style.display = 'block';  \
        } else if(mode == 'e' || func == \"eshelf-\" || func == \"eshelf-1-select-all\" ||  func == \"eshelf-1\" || func == \"eshelf-2\" || func == \"eshelf-new\" || func == \"eshelf-delete\" || func == \"eshelf-save-as\" || func == \"eshelf-rename\" || func == \"eshelf-1-right\" || func == \"eshelf-2-right\" || func == \"eshelf-1-left\" || func == \"eshelf-2-left\" || func == \"eshelf-select-mul\" || func == \"eshelf-select-mult\" || func == \"my-sets-1-del-new\"){  \
                //document.getElementById('MnuMetaMeuEspaco').style.display = 'block';  \
        } else if( (mode == 'b') || (func.indexOf(\"find-db\") != -1) || func == \"find-db-3-next\" || func == \"find-db-3-previous\"){  \
                document.getElementById('MnuMetaBase').style.display = 'block';  \
        } else if(mode == false){  \
                document.getElementById('MnuMetaBusca').style.display = 'block';  \
        }  \
</script>  \
                <div style=\"float:right\"></div>  \
                <!-- <span class=\"titulo rosa\"><img id=\"imgMetaHeader\" src=\"https://www.example.com/templates/periodicos/images/capes/tit_buscaAcervoPagina.gif\" /></span> -->  \
                <!-- <img id=\"imgMetaHeader\" style=\"display: none;\" src=\"https://www.example.com/templates/periodicos/images/capes/tit_buscaAcervoPagina.gif\"  /> -->  \
                <!-- <span class=\"clearfix\"></span> -->  \
<!--  \
                <div class=\"boxAcervo2\" style=\"margin-left:0px;margin-bottom:0px;heigth:20px\">  \
                        <span class=\"titulo2\" style=\"*heigth:10px;\">  \
                                <a id=\"lnkMn225\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00028?func=meta-1&type=m\" title=\"Encontre os resumos, artigos e livros disponÃ­veis no acervo do Portal de PeriÃ³dicos.\" style=\"background:none; height:auto; font-size:1.3em; font-weight:bold; width:190px; margin:auto; color:#8b8784\" onmouseover=\"javascript:mnuShow('divMn225');lnkShow('divMn225');stop=true;\" onmouseout=\"javascript:stop=false;\">Busca Integrada</a>  \
                                <div class=\"sub\" id=\"divMn225\" style=\"display:none;position:absolute;\">  \
                                        <ul class=\"subNavegacao\">  \
                                                <li><a id=\"lnkMn225\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00029?func=meta-1&type=m\" title=\"Encontre os resumos, artigos e livros disponÃ­veis no acervo do Portal de PeriÃ³dicos.\" style=\"background:none; height:auto; font-size:1.3em; font-weight:bold; width:190px; margin:auto; color:#8b8784\" onmouseover=\"javascript:mnuShow('divMn225');lnkShow('divMn225');stop=true;\" onmouseout=\"javascript:stop=false;\">Busca Integrada</a></li>  \
                                                <li><a id=\"lnkMn225\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00030?func=meta-1&type=m\" title=\"Encontre os resumos, artigos e livros disponÃ­veis no acervo do Portal de PeriÃ³dicos.\" style=\"background:none; height:auto; font-size:1.3em; font-weight:bold; width:190px; margin:auto; color:#8b8784\" onmouseover=\"javascript:mnuShow('divMn225');lnkShow('divMn225');stop=true;\" onmouseout=\"javascript:stop=false;\">Busca Integrada</a></li>  \
                                                <li></li>  \
                                                <li ><a id=\"aMnuMetaBusca\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00031?func=meta-5&type=m&i=0\">Refinar Busca</a></li>  \
                                                <li ><a id=\"aMnuMetaBusca\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00032?func=meta-4&type=m&i=1\">Buscas Anteriores</a></li>  \
                                                <li ><a id=\"aMnuMetaBusca\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00033?func=meta-3&type=m&i=2\">Resultado da Busca</a></li>  \
                                                <li ><a id=\"aMnuMetaBusca\" class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00034?func=meta-1&type=m&i=3\">Busca</a></li>  \
                                        </ul>  \
                                </div>  \
                                &nbsp; | &nbsp;  \
                                <a id=\"lnkMn226\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00035?func=find-db-1&type=b\" title=\"Localize bases referenciais e de texto completo por Ã¡rea de conhecimento ou interesse.\" style=\"background:none; height:auto; font-size:1.3em; font-weight:bold; width:190px; margin:auto; color:#8b8784\" onmouseover=\"javascript:mnuShow('divMn226');lnkShow('lnkMn226');stop=true;\" onmouseout=\"javascript:stop=false;\">Buscar Base</a>  \
                                <div id=\"divMn226\" style=\"display:none;position:absolute;\">  \
                                        <ul class=\"subNavegacao\">  \
                                                <li class=\"fim\"><a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00036?func=find-db-0&i=1\">Resultado da Busca</a></li>  \
                                                <li ><a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00037?func=find-db-1&type=b&i=0\">Busca</a></li>  \
                                        </ul>  \
                                </div>  \
                                &nbsp; | &nbsp;  \
                                <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00038?func=find-ej-1&type=p\" target=\"\" title=\"Localize tÃ­tulos de periÃ³dicos em texto completo por Ã¡rea de conhecimento ou interesse.\" style=\"background:none; height:auto; font-size:1.3em; font-weight:bold; width:190px; margin:auto; color:#8b8784\" onmouseover=\"javascript:mnuShow('divMn227');stop=true;\" onmouseout=\"javascript:stop=false;\">Buscar Periódico</a>  \
                                <div id=\"divMn227\" style=\"display:none;position:absolute;\">  \
                                        <ul class=\"subNavegacao\">  \
                                                <li class=\"fim\"><a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00039?func=find-ej-1&type=p&i=0\">Busca</a></li>  \
                                        </ul>  \
                                </div>  \
                                &nbsp; | &nbsp;  \
                                <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00040?func=eshelf-2&type=e\" title=\"Crie alertas de pesquisa, salve seus artigos, periÃ³dicos e bases referenciais preferidos.\" style=\"background:none; height:auto; font-size:1.3em; font-weight:bold; width:190px; margin:auto; color:#8b8784\" onmouseover=\"javascript:mnuShow('divMn228');stop=true;\" onmouseout=\"javascript:stop=false;\">Meu Espaço</a>  \
                                <div id=\"divMn228\" style=\"display:none;position:absolute;\">  \
                                        <ul class=\"subNavegacao\">  \
                                                <li ><a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00041?func=history-1&type=e&i=0\">Minhas EstratÃ©gias de Busca</a></li>  \
                                                <li ><a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00042?func=find-ejournal-1&type=e&i=1\">Meus PeriÃ³dicos</a></li>  \
                                                <li ><a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00043?func=my-sets-1&type=e&i=2\">Meus Conjuntos de Bases</a></li>  \
                                                <li ><a class=\"\" onmouseover=\"javascript:stop=true;\" onmouseout=\"javascript:stop=false;\" href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00044?func=eshelf-2&type=e&i=3\">Meus Artigos</a></li>  \
                                        </ul>  \
                                </div>  \
                        </span>  \
                </div>  \
        </div>  \
  \
        <br />  \
        <br />  \
        <script>  \
        if(mode == 'm'){  \
                mnuShow('divMn225');  \
                setClassLinks('divMn225');  \
        } else if(mode == 'b'){  \
                mnuShow('divMn226');  \
                setClassLinks('divMn226');  \
        } else if(mode == 'p'){  \
                mnuShow('divMn227');  \
                setClassLinks('divMn227');  \
        } else if(mode == 'e'){  \
                mnuShow('divMn228');  \
                setClassLinks('divMn228');  \
        }  \
        </script>  \
  -->  \
  \
  \
  \
  \
  <div id=\"navMenu\" style=\"display:none\">  \
    <div id=\"navigation\" style=\"background-color:#62a0ca\">  \
      <ul id=\"navbar\">  \
        <li id=\"navigationQS\" class=\"TopTB\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00045?func=quick-1\" title=\"Busca Rápida\" accesskey=\"1\">Busca Rápida</a>  \
        </li>  \
        <li id=\"navigationFD\" class=\"NavSelect\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00046?func=find-db-1\" title=\"Buscar Base\" accesskey=\"2\">Buscar Base</a>  \
        </li>  \
        <li id=\"navigationFE\" class=\"TopTB\">  \
        <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00047?func=find-ej-1\" title=\"Buscar Periódico\" accesskey=\"3\" target=\"\">Buscar Periódico</a>  \
        </li>  \
        <li id=\"navigationMS\" class=\"TopTB\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00048?func=meta-1\" title=\"Busca Integrada\" accesskey=\"4\">Busca Integrada</a>  \
        </li>  \
        <li id=\"navigationMy\" class=\"lastItem TopTB\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00049?func=eshelf-2\" title=\"Meu Espaço\" accesskey=\"5\">Meu Espaço</a>  \
        </li>  \
      </ul>  \
    </div>  \
    <div id=\"actionIcons\">  \
      <ul>  \
        <li id=\"actionIconsLang\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00050?func=menu-lng\" title=\"Mudar Idioma\" accesskey=\"6\" onclick=\"javascript:loadmenu('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00051?func=menu-lng',125,150,'right'); return false;\"><img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-language.png\" alt=\"Idioma\" title=\"Idioma\"><span class=\"ActionText\">Idioma</span></a>  \
        </li>  \
        <li id=\"actionIconsEnv\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00052?func=menu-portal\" title=\"Mudar Portal\" accesskey=\"7\" onclick=\"javascript:loadmenu('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00053?func=menu-portal',125,150,'right'); return false;\"><img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-environment.png\" alt=\"Portal\" title=\"Portal\"><span class=\"ActionText\">Portal</span></a>  \
        </li>  \
        <li id=\"actionIconsLogin\">  \
          <a href=\"https://buscador.example.com/pds?func=load-login&amp;calling_system=metalib&amp;institute=CAPES&amp;url=http://buscador.example.com:80/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00054?func=find-db-0\" title=\"Identificação\" accesskey=\"8\"><img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-login.png\" alt=\"Identificar-se\" title=\"Identificar-se\"><span class=\"ActionText\">Identificação</span></a>  \
        </li>  \
        <li id=\"actionIconsLogout\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00055?func=file&amp;file_name=end-session-confirm&amp;back_func=find-db-0\" title=\"Encerrar sessão\" accesskey=\"e\" onclick=\"javascript:loadmenu('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00056?func=file&amp;file_name=end-session-confirm', 270, 150); return false;\"><img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-ico_exit.png\" alt=\"Encerrar sessão\" title=\"Encerrar sessão\"><span class=\"ActionText\">Encerrar sessão</span></a>  \
        </li>  \
        <li id=\"actionIconsHelp\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00057?func=help&amp;file_name=FIND-DB&amp;section=list%27\" title=\"Ajuda\" accesskey=\"h\" onclick=\"javascript:open_window_help('https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00058?func=help&amp;file_name=FIND-DB&amp;section=list');return false;\"><img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-help.png\" alt=\"Ajuda\" title=\"Ajuda\"><span class=\"ActionText\">Ajuda</span></a>  \
        </li>  \
      </ul>  \
    </div>  \
  </div>  \
<!-- FILE NAME: /exlibris/metalib/m4_1/inovo/www_v_por/find-db-toolbar --> \
<!-- START_SECTION --><!-- db-not-selected -->  \
  \
        <script language=\"javascript\">  \
                document.getElementById('imgMetaHeader').src = 'https://www.example.com/templates/periodicos/images/capes/tit_buscaBasePagina.gif';  \
  \
                //document.getElementById('pMnuMetaBusca').style.display = 'block';  \
                //document.getElementById('pMnuDatabase').style.display = 'none';  \
                //document.getElementById('pMnuSFX').style.display = 'block';  \
                //document.getElementById('pMnuMeuEspaco').style.display = 'block';  \
  \
                //document.getElementById('aMnuDatabaseBusca').style.display = 'block';  \
                //document.getElementById('aMnuDatabaseResultado').style.display = 'block';  \
  \
                //document.getElementById('aMnuSFXBusca').style.display = 'none';  \
  \
                //document.getElementById('aMnuMetaBusca').style.display = 'none';  \
                //document.getElementById('aMnuMetaResultado').style.display = 'none';  \
                //document.getElementById('aMnuMetaAnteriores').style.display = 'none';  \
                //document.getElementById('aMnuMetaRefinar').style.display = 'none';  \
  \
                //document.getElementById('aMnuDatabaseBusca').href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00059?func=find-db-1';  \
                //document.getElementById('aMnuDatabaseResultado').href = 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00060?func=find-db-0';  \
        </script>  \
  \
  \
<!--  \
    <div id=\"toolbar\">  \
    <ul>  \
      <li id=\"toolbarFD\" class=\"TB\">  \
        <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00061?func=find-db-1\" title=\"Buscar Base\">Buscar Base</a>  \
      </li>  \
      <li id=\"toolbarDL\" class=\"lastItem ToolbarSelect\">  \
        <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00062?func=find-db-0\" title=\"Lista de Bases\">Lista de Bases</a>  \
      </li>  \
      </ul>  \
    </div>  \
 -->  \
<!-- END_SECTION --><!-- db-not-selected -->  \
<!-- FILE NAME: /exlibris/metalib/m4_1/inovo/www_v_por/toolbar-close --> \
<!-- toolbar-close -->  \
  \
<!-- FILE NAME: /exlibris/metalib/m4_1/inovo/www_v_por/find-db-3-body --> \
<!-- START_SECTION --><!-- head -->  \
</div>  \
<a name=\"startcontent\" id=\"startcontent\"></a>  \
<div id=\"contentwrap\" style=\"width:98%\">  \
<div class=\"homeConteudoPrincipal\" style=\"width:100%\">  \
<!--  \
    <h1>Lista de Bases</h1><span id=\"message\" ></span>  \
-->  \
        <script type=\"text/javascript\" language=\"JavaScript\">  \
        document.getElementById('iMnuDatabaseBusca').src = '//JoomlaUrl/templates/periodicos/images/capes/btn_busca.gif';  \
                document.getElementById('iMnuDatabaseResultado').src = '//JoomlaUrl/templates/periodicos/images/capes/btn_resultadoBusca_ativo.gif';  \
  \
        function linkVar(div,retornar){  \
            link = document.getElementById(div).innerHTML;  \
            link = link.replace(\"\'\",\"\\\"\");  \
            link = link.replace(\"&amp;\",\"&\");  \
            link = link.split(\"\\\"\\\");  \
            eLink = 0;  \
            for(cont=0; cont<link.length; cont++){  \
                if(link[cont].indexOf('http') >= 0){  \
                        impLink = link[cont];  \
                        eLink++;  \
                }  \
            }  \
            if(eLink == 0){  \
                impLink = 'vazio';  \
            }  \
            if(retornar == 'imprime'){  \
                        document.write(impLink);  \
                }else{  \
                        return impLink;  \
                }  \
        }  \
  \
            function redirect(div){  \
                                url = linkVar(div, 'retorna');  \
                                if (url != 'vazio') location.href = url;  \
        }  \
  \
        function verifyChecked(){  \
                link1Var = linkVar('link1','retorna');  \
                link2Var = linkVar('link2','retorna');  \
                if(link1Var.indexOf('http') < 0){  \
                        document.getElementById('check1').checked = 'checked';  \
                }  \
                if(link2Var.indexOf('http') < 0){  \
                        document.getElementById('check2').checked = 'checked';  \
                }  \
        }  \
    </script>  \
<script type=\"text/javascript\">  \
        if( mode == null || mode == 'm' ) {  \
                document.getElementById('MnuMetaBase').style.display = 'block';  \
        }  \
</script>  \
    <div id=\"link1\" style=\"display:none\">Lista de resultados</div>  \
        <div id=\"link2\" style=\"display:none\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00063?func=find-db-3&amp;format=001\" title=\"Lista detalhada de resultados\">Lista detalhada de resultados</a></div>  \
  \
        <div class=\"fitroEstatistica acervo\" style=\"margin-bottom:5px;\">  \
        <!-- div class=\"span12\">  \
        <h1 style=\"font-size: 40px\">  \
                <b>Buscar base</b>  \
                <small style=\"font-size: 15px;\">(Resultado da busca)</small>  \
        </h1>  \
        </div -->  \
                <!--  \
                 <span class=\"tituloAcervo\">Buscar base&nbsp;/&nbsp;</span>  \
                 <span style=\"line-height: 25px; margin-left: 0px; float:left\">Resultado da busca</span>  \
                 <div style='float:left; margin-left:250px;'>  \
  \
                        <label style='float:left;padding-right:30px;'><input id='check1' type='radio' name='formato_tabela' onclick='redirect(\"link1\")'>Lista de resultados</label>  \
                        <label style='float:left;'><input id='check2' type='radio' name='formato_tabela' onclick='redirect(\"link2\")'>Lista detalhada de resultados</label>  \
                        <script>verifyChecked();</script>  \
                -->  \
                 </div>  \
                 <div style=\"clear:both\"></div>  \
        </div>  \
  <div class=\"record_list_header\">  \
        <div class=\"containerTop\" style=\"height:100%;\">  \
          <div class=\"alignLeft\">Você buscou por \"<strong>Lista A a Z = *</strong>\" <!-- - Localizados <strong>      256</strong> Bases &nbsp;&nbsp;&nbsp;&nbsp; -->  \
          <br>  \
                  <script>  \
  \
                                var total = Math.ceil(      256 / 30);  \
                                var pag = 1;  \
  \
                                var url = document.URL;  \
  \
                                if(url.indexOf('set-entry') != -1) {  \
                                        var pag = Math.ceil((url.substring(url.indexOf('set-entry=') + 10) -1 )/ 30);  \
  \
                                        if( url.indexOf('find-db-3-previous') != -1 ){  \
                                                //pag -= 2;  \
                                        }else if( url.indexOf('find-db-3-next') != -1 ){  \
                                                pag += 2;  \
                                        }  \
  \
                                }  \
  \
                                function pagNavigation() {  \
                                        document.write('Página: ' + pag + ' de ' + total);  \
                                }  \
  \
                                function pagNavigationFirst() {  \
                                        if(url.indexOf('set-entry=') == -1){  \
                                                document.write('&lt;&lt;');  \
                                                return;  \
                                        }  \
  \
                                        if( url.indexOf('find-db-3-previous') != -1 && parseInt(url.substring(url.indexOf('set-entry=') + 10)) != 31 ){  \
                                                document.write('&lt;&lt;');  \
                                        }else{  \
                                                document.write('<a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00064?func=find-db-3-previous&set-entry=000031\">&lt;&lt;</a>');  \
                                        }  \
                                }  \
  \
                                function pagNavigationLast() {  \
                                        if( total == 1 ){  \
                                                document.write('&gt;&gt;');  \
                                                return;  \
                                        }  \
  \
                                        totalReg = (total-2) * 30 + 1;  \
  \
                                        if(totalReg <= 0) totalReg = 1;  \
  \
                                        if( url.indexOf('set-entry=') == -1 || parseInt(url.substring(url.indexOf('set-entry=') + 10)) !=       256 && url.indexOf('find-db-3-previous') != -1 ){  \
                                                document.write('<a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00065?func=find-db-3-next&set-entry=0000'+(totalReg)+'\">&gt;&gt;</a>');  \
                                        }else{  \
                                                document.write('&gt;&gt;');  \
                                        }  \
                                }  \
  \
                                var listaAtual = (pag == 1)? 1 : ((pag-1) * 30 + 1);  \
  \
                                function baseAtual() {  \
  \
                                        if(      256 <= listaAtual + 29) {  \
                                                document.write('' + listaAtual + ' - ' +       256 + ' de ' +       256 + ' Base(s)');  \
                                        }else{  \
                                                document.write('' + listaAtual + ' - ' + (listaAtual + 29) + ' de ' +       256 + ' Base(s)');  \
                                        }  \
  \
                                }  \
                                baseAtual();  \
  \
                  </script>1 - 30 de 256 Base(s)  \
          </div>  \
                <div class=\"alignRight\">  \
                <span class=\"no_wrap\"><strong> </strong></span>  \
                </div>  \
        </div>  \
        <!--  \
        <div class=\"containerMiddle\">  \
                    <ul>  \
                            <li>Lista de resultados  \
                            </li>  \
                            <li><A HREF=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00066?func=find-db-3&amp;format=001\" title= 'Lista detalhada de resultados'>Lista detalhada de resultados</A>  \
                            </li>  \
                    </ul>  \
        </div>  \
        -->  \
<!-- END_SECTION --><!-- head -->  \
<!-- START_SECTION --><!-- head-expand -->  \
</div>  \
 \
<!-- START INCLUDE FILE NAME:/exlibris/metalib/m4_1/inovo/www_v_por/find-db-3-nav-toolbar --> \
  \
<!--  \
<div class=\"record_list_header\">  \
  <div class=\"containerBottom\">  \
        <div class=\"alignLeft\">  \
                <div class=\"record_list_header_left\">  \
                        <strong>19th - Biblioteca</strong>  \
                </div>  \
                <div class=\"record_list_header_right\">  \
                        Ir para <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00067?func=meta-1&amp;init_type=Y\" title=\"Busca Integrada\" class=\"metaSearch\">Busca Integrada</a>  \
                </div>  \
        </div>  \
        <div class=\"alignRight\">  \
                <span class=\"no_wrap\"><strong><span class=\"fade\">&lt;Anterior</span> <a href='https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00068?func=find-db-3-next&amp;set-entry=000001' title='Próxima Página'>Próximo&gt;</a></strong></span>  \
                </div>  \
  </div>  \
</div>  \
-->  \
  \
<div name=\"basePag\" style=\"float:left; margin-top:-20px; display:none;\">  \
        <script>  \
                baseAtual();  \
        </script>1 - 30 de 256 Base(s)  \
</div>  \
  \
<script>  \
        if(document.getElementsByName('basePag').length == 2) {  \
                document.getElementsByName('basePag')[1].style.display = 'block';  \
        }  \
</script>  \
  \
<div style=\"float:right; margin-top:-20px;\">  \
        <span class=\"no_wrap\"><strong><script>pagNavigationFirst();</script>&lt;&lt; <span class=\"fade\">&lt;Anterior</span> | <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00069?func=find-db-3-next&amp;set-entry=000001\" title=\"Próxima Página\">Próximo&gt;</a> <script>pagNavigationLast();</script><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00065?func=find-db-3-next&amp;set-entry=0000211\">&gt;&gt;</a></strong></span>  \
        <br>  \
        <script>  \
                pagNavigation();  \
        </script>Página: 1 de 9  \
</div>  \
 \
<!-- END OF INCLUDE FILE:/exlibris/metalib/m4_1/inovo/www_v_por/find-db-3-nav-toolbar --> \
  \
<!-- END_SECTION --><!-- head-expand -->  \
<!-- START_SECTION --><!-- body-table-head -->  \
<br>  \
<div id=\"record_list\" style=\"width:100%\">  \
  <table style=\"margin-bottom:5px; width:100%;\" cellspacing=\"0\">  \
    <tbody><tr>  \
      <th scope=\"col\" style=\"width:10px\" align=\"left\">&nbsp;</th>  \
      <th scope=\"col\" style=\"width:60%\" align=\"left\">Nome da base</th>  \
      <th scope=\"col\" style=\"width:30%\" align=\"left\">Tipo</th>  \
      <th scope=\"col\" style=\"width:10%\" align=\"left\">Ações</th>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-head -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00070?func=native-link&amp;resource=CAP03601\" title=\"19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)\" target=\"_blank\">19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00071?func=find-db-info&amp;doc_num=000006650\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00072?func=find-db-info&amp;doc_num=000006650','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)\" title=\"Informação: 19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00073?func=find-db-add-res&amp;resource=CAP03601&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03601\",\"000000000\",\"myImageCAP03601\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00074\");return false;'><img name=\"myImageCAP03601\" id=\"myImageCAP03601\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-body-1 -->  \
        <!-- <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00075?func=find-db-4&amp;resource=CAP03601\"><img src=\"https://buscador.example.com/INOVO/icon_por/v-search_more2.png\" alt=\"Busca na Base: 19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)\" title=\"Busca na Base: 19th Century: Asia and the West: Diplomacy and Cultural Exchange (Gale)\" style=\"margin-bottom:0;\"></a> -->  \
<!-- END_SECTION --><!-- body-table-body-1 -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00076?func=native-link&amp;resource=CAP03795\" title=\"19th Century : British Politics and Society (Gale)\" target=\"_blank\">19th Century : British Politics and Society (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00077?func=find-db-info&amp;doc_num=000006979\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00078?func=find-db-info&amp;doc_num=000006979','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century : British Politics and Society (Gale)\" title=\"Informação: 19th Century : British Politics and Society (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00079?func=find-db-add-res&amp;resource=CAP03795&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03795\",\"000000000\",\"myImageCAP03795\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00080\");return false;'><img name=\"myImageCAP03795\" id=\"myImageCAP03795\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century : British Politics and Society (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century : British Politics and Society (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00081?func=native-link&amp;resource=CAP03796\" title=\"19th Century: British Theatre, Music and Literature (Gale)\" target=\"_blank\">19th Century: British Theatre, Music and Literature (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos , Audiovisuais&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00082?func=find-db-info&amp;doc_num=000006980\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00083?func=find-db-info&amp;doc_num=000006980','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century: British Theatre, Music and Literature (Gale)\" title=\"Informação: 19th Century: British Theatre, Music and Literature (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00084?func=find-db-add-res&amp;resource=CAP03796&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03796\",\"000000000\",\"myImageCAP03796\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00085\");return false;'><img name=\"myImageCAP03796\" id=\"myImageCAP03796\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: British Theatre, Music and Literature (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: British Theatre, Music and Literature (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00086?func=native-link&amp;resource=CAP03802\" title=\"19th Century: Europe and Africa: Commerce, Christianity, Civilization and Conquest (Gale)\" target=\"_blank\">19th Century: Europe and Africa: Commerce, Christianity, Civilization and Conquest (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00087?func=find-db-info&amp;doc_num=000006986\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00088?func=find-db-info&amp;doc_num=000006986','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century: Europe and Africa: Commerce, Christianity, Civilization and Conquest (Gale)\" title=\"Informação: 19th Century: Europe and Africa: Commerce, Christianity, Civilization and Conquest (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00089?func=find-db-add-res&amp;resource=CAP03802&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03802\",\"000000000\",\"myImageCAP03802\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00090\");return false;'><img name=\"myImageCAP03802\" id=\"myImageCAP03802\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Europe and Africa: Commerce, Christianity, Civilization and Conquest (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Europe and Africa: Commerce, Christianity, Civilization and Conquest (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00091?func=native-link&amp;resource=CAP03797\" title=\"19th Century : European Literature, 1790-1840: the Corvey Collection (Gale)\" target=\"_blank\">19th Century : European Literature, 1790-1840: the Corvey Collection (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00092?func=find-db-info&amp;doc_num=000006981\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00093?func=find-db-info&amp;doc_num=000006981','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century : European Literature, 1790-1840: the Corvey Collection (Gale)\" title=\"Informação: 19th Century : European Literature, 1790-1840: the Corvey Collection (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00094?func=find-db-add-res&amp;resource=CAP03797&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03797\",\"000000000\",\"myImageCAP03797\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00095\");return false;'><img name=\"myImageCAP03797\" id=\"myImageCAP03797\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century : European Literature, 1790-1840: the Corvey Collection (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century : European Literature, 1790-1840: the Corvey Collection (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00096?func=native-link&amp;resource=CAP03801\" title=\"19th Century: Photography: The World Through the Lens (Gale)\" target=\"_blank\">19th Century: Photography: The World Through the Lens (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Audiovisuais , Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00097?func=find-db-info&amp;doc_num=000006985\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00098?func=find-db-info&amp;doc_num=000006985','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century: Photography: The World Through the Lens (Gale)\" title=\"Informação: 19th Century: Photography: The World Through the Lens (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00099?func=find-db-add-res&amp;resource=CAP03801&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03801\",\"000000000\",\"myImageCAP03801\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00100\");return false;'><img name=\"myImageCAP03801\" id=\"myImageCAP03801\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Photography: The World Through the Lens (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Photography: The World Through the Lens (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00101?func=native-link&amp;resource=CAP03798\" title=\"19th Century: Science, Technology and Medicine, 1780-1925 (Gale)\" target=\"_blank\">19th Century: Science, Technology and Medicine, 1780-1925 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00102?func=find-db-info&amp;doc_num=000006982\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00103?func=find-db-info&amp;doc_num=000006982','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century: Science, Technology and Medicine, 1780-1925 (Gale)\" title=\"Informação: 19th Century: Science, Technology and Medicine, 1780-1925 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00104?func=find-db-add-res&amp;resource=CAP03798&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03798\",\"000000000\",\"myImageCAP03798\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00105\");return false;'><img name=\"myImageCAP03798\" id=\"myImageCAP03798\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Science, Technology and Medicine, 1780-1925 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Science, Technology and Medicine, 1780-1925 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00106?func=native-link&amp;resource=CAP03800\" title=\"19th Century: Women: Transnational Networks (Gale)\" target=\"_blank\">19th Century: Women: Transnational Networks (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00107?func=find-db-info&amp;doc_num=000006984\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00108?func=find-db-info&amp;doc_num=000006984','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: 19th Century: Women: Transnational Networks (Gale)\" title=\"Informação: 19th Century: Women: Transnational Networks (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00109?func=find-db-add-res&amp;resource=CAP03800&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03800\",\"000000000\",\"myImageCAP03800\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00110\");return false;'><img name=\"myImageCAP03800\" id=\"myImageCAP03800\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Women: Transnational Networks (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: 19th Century: Women: Transnational Networks (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00111?func=native-link&amp;resource=CAP02988\" title=\"ADI - Africa development indicators\" target=\"_blank\">ADI - Africa development indicators</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Estatísticas&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00112?func=find-db-info&amp;doc_num=000005493\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00113?func=find-db-info&amp;doc_num=000005493','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: ADI - Africa development indicators\" title=\"Informação: ADI - Africa development indicators\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00114?func=find-db-add-res&amp;resource=CAP02988&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02988\",\"000000000\",\"myImageCAP02988\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00115\");return false;'><img name=\"myImageCAP02988\" id=\"myImageCAP02988\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: ADI - Africa development indicators\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: ADI - Africa development indicators\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-body-1 -->  \
        <!-- <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00116?func=find-db-4&amp;resource=CAP02988\"><img src=\"https://buscador.example.com/INOVO/icon_por/v-search_more2.png\" alt=\"Busca na Base: ADI - Africa development indicators\" title=\"Busca na Base: ADI - Africa development indicators\" style=\"margin-bottom:0;\"></a> -->  \
<!-- END_SECTION --><!-- body-table-body-1 -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00117?func=native-link&amp;resource=CAP02676\" title=\"AGRICOLA : NAL Catalog\" target=\"_blank\">AGRICOLA : NAL Catalog</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Referenciais com resumos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00118?func=find-db-info&amp;doc_num=000004997\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00119?func=find-db-info&amp;doc_num=000004997','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: AGRICOLA : NAL Catalog\" title=\"Informação: AGRICOLA : NAL Catalog\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00120?func=find-db-add-res&amp;resource=CAP02676&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02676\",\"000000000\",\"myImageCAP02676\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00121\");return false;'><img name=\"myImageCAP02676\" id=\"myImageCAP02676\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: AGRICOLA : NAL Catalog\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: AGRICOLA : NAL Catalog\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-body-1 -->  \
        <!-- <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00122?func=find-db-4&amp;resource=CAP02676\"><img src=\"https://buscador.example.com/INOVO/icon_por/v-search_more2.png\" alt=\"Busca na Base: AGRICOLA : NAL Catalog\" title=\"Busca na Base: AGRICOLA : NAL Catalog\" style=\"margin-bottom:0;\"></a> -->  \
<!-- END_SECTION --><!-- body-table-body-1 -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00123?func=native-link&amp;resource=CAP02674\" title=\"AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)\" target=\"_blank\">AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Referenciais com resumos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00124?func=find-db-info&amp;doc_num=000004995\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00125?func=find-db-info&amp;doc_num=000004995','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)\" title=\"Informação: AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00126?func=find-db-add-res&amp;resource=CAP02674&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02674\",\"000000000\",\"myImageCAP02674\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00127\");return false;'><img name=\"myImageCAP02674\" id=\"myImageCAP02674\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-body-1 -->  \
        <!-- <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00128?func=find-db-4&amp;resource=CAP02674\"><img src=\"https://buscador.example.com/INOVO/icon_por/v-search_more2.png\" alt=\"Busca na Base: AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)\" title=\"Busca na Base: AGRIS : International Information System for the Agricultural Sciences and Technology (FAO)\" style=\"margin-bottom:0;\"></a> -->  \
<!-- END_SECTION --><!-- body-table-body-1 -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00129?func=native-link&amp;resource=CAP02707\" title=\"Alianza de Servicios de Información Agropecuaria - SIDALC\" target=\"_blank\">Alianza de Servicios de Información Agropecuaria - SIDALC</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Referenciais com resumos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00130?func=find-db-info&amp;doc_num=000005028\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00131?func=find-db-info&amp;doc_num=000005028','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Alianza de Servicios de Información Agropecuaria - SIDALC\" title=\"Informação: Alianza de Servicios de Información Agropecuaria - SIDALC\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00132?func=find-db-add-res&amp;resource=CAP02707&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02707\",\"000000000\",\"myImageCAP02707\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00133\");return false;'><img name=\"myImageCAP02707\" id=\"myImageCAP02707\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Alianza de Servicios de Información Agropecuaria - SIDALC\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Alianza de Servicios de Información Agropecuaria - SIDALC\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00134?func=native-link&amp;resource=CAP03794\" title=\"Archives Unbound: Conditions &amp; Politics in Occupied Western Europe 1940-1945 (Gale)\" target=\"_blank\">Archives Unbound: Conditions &amp; Politics in Occupied Western Europe 1940-1945 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00135?func=find-db-info&amp;doc_num=000006978\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00136?func=find-db-info&amp;doc_num=000006978','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: Conditions &amp; Politics in Occupied Western Europe 1940-1945 (Gale)\" title=\"Informação: Archives Unbound: Conditions &amp; Politics in Occupied Western Europe 1940-1945 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00137?func=find-db-add-res&amp;resource=CAP03794&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03794\",\"000000000\",\"myImageCAP03794\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00138\");return false;'><img name=\"myImageCAP03794\" id=\"myImageCAP03794\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Conditions &amp; Politics in Occupied Western Europe 1940-1945 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Conditions &amp; Politics in Occupied Western Europe 1940-1945 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00139?func=native-link&amp;resource=CAP03803\" title=\"Archives Unbound: East Germany from Stalinization to the New Economic Policy 1950-1963 (Gale)\" target=\"_blank\">Archives Unbound: East Germany from Stalinization to the New Economic Policy 1950-1963 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00140?func=find-db-info&amp;doc_num=000006987\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00141?func=find-db-info&amp;doc_num=000006987','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: East Germany from Stalinization to the New Economic Policy 1950-1963 (Gale)\" title=\"Informação: Archives Unbound: East Germany from Stalinization to the New Economic Policy 1950-1963 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00142?func=find-db-add-res&amp;resource=CAP03803&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03803\",\"000000000\",\"myImageCAP03803\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00143\");return false;'><img name=\"myImageCAP03803\" id=\"myImageCAP03803\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: East Germany from Stalinization to the New Economic Policy 1950-1963 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: East Germany from Stalinization to the New Economic Policy 1950-1963 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00144?func=native-link&amp;resource=CAP03804\" title=\"Archives Unbound: Foreign Relations between Latin America and the Caribbean States 1930-1944(Gale)\" target=\"_blank\">Archives Unbound: Foreign Relations between Latin America and the Caribbean States 1930-1944(Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00145?func=find-db-info&amp;doc_num=000006988\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00146?func=find-db-info&amp;doc_num=000006988','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: Foreign Relations between Latin America and the Caribbean States 1930-1944(Gale)\" title=\"Informação: Archives Unbound: Foreign Relations between Latin America and the Caribbean States 1930-1944(Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00147?func=find-db-add-res&amp;resource=CAP03804&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03804\",\"000000000\",\"myImageCAP03804\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00148\");return false;'><img name=\"myImageCAP03804\" id=\"myImageCAP03804\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Foreign Relations between Latin America and the Caribbean States 1930-1944(Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Foreign Relations between Latin America and the Caribbean States 1930-1944(Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00149?func=native-link&amp;resource=CAP03805\" title=\"Archives Unbound: Post-War Europe: Refugees, Exile and Resettlement, 1945-1950 (Gale)\" target=\"_blank\">Archives Unbound: Post-War Europe: Refugees, Exile and Resettlement, 1945-1950 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00150?func=find-db-info&amp;doc_num=000006989\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00151?func=find-db-info&amp;doc_num=000006989','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: Post-War Europe: Refugees, Exile and Resettlement, 1945-1950 (Gale)\" title=\"Informação: Archives Unbound: Post-War Europe: Refugees, Exile and Resettlement, 1945-1950 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00152?func=find-db-add-res&amp;resource=CAP03805&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03805\",\"000000000\",\"myImageCAP03805\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00153\");return false;'><img name=\"myImageCAP03805\" id=\"myImageCAP03805\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Post-War Europe: Refugees, Exile and Resettlement, 1945-1950 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Post-War Europe: Refugees, Exile and Resettlement, 1945-1950 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00154?func=native-link&amp;resource=CAP03806\" title=\"Archives Unbound: Records of the National Council for United States-China Trade 1973-1983 (Gale)\" target=\"_blank\">Archives Unbound: Records of the National Council for United States-China Trade 1973-1983 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00155?func=find-db-info&amp;doc_num=000006990\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00156?func=find-db-info&amp;doc_num=000006990','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: Records of the National Council for United States-China Trade 1973-1983 (Gale)\" title=\"Informação: Archives Unbound: Records of the National Council for United States-China Trade 1973-1983 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00157?func=find-db-add-res&amp;resource=CAP03806&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03806\",\"000000000\",\"myImageCAP03806\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00158\");return false;'><img name=\"myImageCAP03806\" id=\"myImageCAP03806\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Records of the National Council for United States-China Trade 1973-1983 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Records of the National Council for United States-China Trade 1973-1983 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00159?func=native-link&amp;resource=CAP03807\" title=\"Archives Unbound: Testaments to the Holocaust (Gale)\" target=\"_blank\">Archives Unbound: Testaments to the Holocaust (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00160?func=find-db-info&amp;doc_num=000006991\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00161?func=find-db-info&amp;doc_num=000006991','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: Testaments to the Holocaust (Gale)\" title=\"Informação: Archives Unbound: Testaments to the Holocaust (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00162?func=find-db-add-res&amp;resource=CAP03807&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03807\",\"000000000\",\"myImageCAP03807\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00163\");return false;'><img name=\"myImageCAP03807\" id=\"myImageCAP03807\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Testaments to the Holocaust (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Testaments to the Holocaust (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00164?func=native-link&amp;resource=CAP03808\" title=\"Archives Unbound: The Middle East Online: Arab-Israeli Relations 1917-1970 (Gale)\" target=\"_blank\">Archives Unbound: The Middle East Online: Arab-Israeli Relations 1917-1970 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00165?func=find-db-info&amp;doc_num=000006992\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00166?func=find-db-info&amp;doc_num=000006992','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: The Middle East Online: Arab-Israeli Relations 1917-1970 (Gale)\" title=\"Informação: Archives Unbound: The Middle East Online: Arab-Israeli Relations 1917-1970 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00167?func=find-db-add-res&amp;resource=CAP03808&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03808\",\"000000000\",\"myImageCAP03808\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00168\");return false;'><img name=\"myImageCAP03808\" id=\"myImageCAP03808\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: The Middle East Online: Arab-Israeli Relations 1917-1970 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: The Middle East Online: Arab-Israeli Relations 1917-1970 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00169?func=native-link&amp;resource=CAP03809\" title=\"Archives Unbound: The Middle East Online: Iraq 1914-1974 (Gale)\" target=\"_blank\">Archives Unbound: The Middle East Online: Iraq 1914-1974 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00170?func=find-db-info&amp;doc_num=000006993\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00171?func=find-db-info&amp;doc_num=000006993','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: The Middle East Online: Iraq 1914-1974 (Gale)\" title=\"Informação: Archives Unbound: The Middle East Online: Iraq 1914-1974 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00172?func=find-db-add-res&amp;resource=CAP03809&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03809\",\"000000000\",\"myImageCAP03809\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00173\");return false;'><img name=\"myImageCAP03809\" id=\"myImageCAP03809\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: The Middle East Online: Iraq 1914-1974 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: The Middle East Online: Iraq 1914-1974 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00174?func=native-link&amp;resource=CAP03810\" title=\"Archives Unbound: Women, War and Society 1914-1918 (Gale)\" target=\"_blank\">Archives Unbound: Women, War and Society 1914-1918 (Gale)</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Textos completos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00175?func=find-db-info&amp;doc_num=000006994\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00176?func=find-db-info&amp;doc_num=000006994','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Archives Unbound: Women, War and Society 1914-1918 (Gale)\" title=\"Informação: Archives Unbound: Women, War and Society 1914-1918 (Gale)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00177?func=find-db-add-res&amp;resource=CAP03810&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP03810\",\"000000000\",\"myImageCAP03810\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00178\");return false;'><img name=\"myImageCAP03810\" id=\"myImageCAP03810\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Women, War and Society 1914-1918 (Gale)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Archives Unbound: Women, War and Society 1914-1918 (Gale)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00179?func=native-link&amp;resource=CAP02334\" title=\"ArXiv.org\" target=\"_blank\">ArXiv.org</a></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Arquivos Abertos e Redes de e-prints&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00180?func=find-db-info&amp;doc_num=000004655\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00181?func=find-db-info&amp;doc_num=000004655','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: ArXiv.org\" title=\"Informação: ArXiv.org\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00182?func=find-db-add-res&amp;resource=CAP02334&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02334\",\"000000000\",\"myImageCAP02334\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00183\");return false;'><img name=\"myImageCAP02334\" id=\"myImageCAP02334\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: ArXiv.org\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: ArXiv.org\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-body-1 -->  \
        <!-- <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00184?func=find-db-4&amp;resource=CAP02334\"><img src=\"https://buscador.example.com/INOVO/icon_por/v-search_more2.png\" alt=\"Busca na Base: ArXiv.org\" title=\"Busca na Base: ArXiv.org\" style=\"margin-bottom:0;\"></a> -->  \
<!-- END_SECTION --><!-- body-table-body-1 -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00185?func=native-link&amp;resource=CAP04258\" title=\"Banco de Objetos de Metrologia (BOM). Repositório Institucional\" target=\"_blank\">Banco de Objetos de Metrologia (BOM). Repositório Institucional</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Repositórios Institucionais&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00186?func=find-db-info&amp;doc_num=000007760\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00187?func=find-db-info&amp;doc_num=000007760','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Banco de Objetos de Metrologia (BOM). Repositório Institucional\" title=\"Informação: Banco de Objetos de Metrologia (BOM). Repositório Institucional\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00188?func=find-db-add-res&amp;resource=CAP04258&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP04258\",\"000000000\",\"myImageCAP04258\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00189\");return false;'><img name=\"myImageCAP04258\" id=\"myImageCAP04258\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Banco de Objetos de Metrologia (BOM). Repositório Institucional\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Banco de Objetos de Metrologia (BOM). Repositório Institucional\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00190?func=native-link&amp;resource=CAP02571\" title=\"Banco Nacional de Desenvolvimento Econômico e Social (BNDES). Estudos e Publicações\" target=\"_blank\">Banco Nacional de Desenvolvimento Econômico e Social (BNDES). Estudos e Publicações</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Outras Fontes&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00191?func=find-db-info&amp;doc_num=000004892\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00192?func=find-db-info&amp;doc_num=000004892','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Banco Nacional de Desenvolvimento Econômico e Social (BNDES). Estudos e Publicações\" title=\"Informação: Banco Nacional de Desenvolvimento Econômico e Social (BNDES). Estudos e Publicações\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00193?func=find-db-add-res&amp;resource=CAP02571&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02571\",\"000000000\",\"myImageCAP02571\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00194\");return false;'><img name=\"myImageCAP02571\" id=\"myImageCAP02571\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Banco Nacional de Desenvolvimento Econômico e Social (BNDES). Estudos e Publicações\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Banco Nacional de Desenvolvimento Econômico e Social (BNDES). Estudos e Publicações\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00195?func=native-link&amp;resource=CAP02673\" title=\"Base Bibliográfica da Agricultura Brasileira : AGROBASE\" target=\"_blank\">Base Bibliográfica da Agricultura Brasileira : AGROBASE</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Referenciais com resumos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00196?func=find-db-info&amp;doc_num=000004994\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00197?func=find-db-info&amp;doc_num=000004994','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Base Bibliográfica da Agricultura Brasileira : AGROBASE\" title=\"Informação: Base Bibliográfica da Agricultura Brasileira : AGROBASE\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00198?func=find-db-add-res&amp;resource=CAP02673&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02673\",\"000000000\",\"myImageCAP02673\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00199\");return false;'><img name=\"myImageCAP02673\" id=\"myImageCAP02673\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Base Bibliográfica da Agricultura Brasileira : AGROBASE\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Base Bibliográfica da Agricultura Brasileira : AGROBASE\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00200?func=native-link&amp;resource=CAP02799\" title=\"Base de Patentes Brasileiras - INPI\" target=\"_blank\">Base de Patentes Brasileiras - INPI</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Patentes&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00201?func=find-db-info&amp;doc_num=000005146\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00202?func=find-db-info&amp;doc_num=000005146','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Base de Patentes Brasileiras - INPI\" title=\"Informação: Base de Patentes Brasileiras - INPI\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00203?func=find-db-add-res&amp;resource=CAP02799&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02799\",\"000000000\",\"myImageCAP02799\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00204\");return false;'><img name=\"myImageCAP02799\" id=\"myImageCAP02799\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Base de Patentes Brasileiras - INPI\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Base de Patentes Brasileiras - INPI\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00205?func=native-link&amp;resource=CAP02675\" title=\"Bases de Dados da Pesquisa Agropecuária EMBRAPA : BDPA\" target=\"_blank\">Bases de Dados da Pesquisa Agropecuária EMBRAPA : BDPA</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Referenciais com resumos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00206?func=find-db-info&amp;doc_num=000004996\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00207?func=find-db-info&amp;doc_num=000004996','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Bases de Dados da Pesquisa Agropecuária EMBRAPA : BDPA\" title=\"Informação: Bases de Dados da Pesquisa Agropecuária EMBRAPA : BDPA\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00208?func=find-db-add-res&amp;resource=CAP02675&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02675\",\"000000000\",\"myImageCAP02675\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00209\");return false;'><img name=\"myImageCAP02675\" id=\"myImageCAP02675\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Bases de Dados da Pesquisa Agropecuária EMBRAPA : BDPA\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Bases de Dados da Pesquisa Agropecuária EMBRAPA : BDPA\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00210?func=native-link&amp;resource=CAP02681\" title=\"Bibliografia Brasileira de Odontologia : BBO\" target=\"_blank\">Bibliografia Brasileira de Odontologia : BBO</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Referenciais com resumos&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00211?func=find-db-info&amp;doc_num=000005002\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00212?func=find-db-info&amp;doc_num=000005002','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Bibliografia Brasileira de Odontologia : BBO\" title=\"Informação: Bibliografia Brasileira de Odontologia : BBO\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00213?func=find-db-add-res&amp;resource=CAP02681&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02681\",\"000000000\",\"myImageCAP02681\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00214\");return false;'><img name=\"myImageCAP02681\" id=\"myImageCAP02681\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Bibliografia Brasileira de Odontologia : BBO\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Bibliografia Brasileira de Odontologia : BBO\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00215?func=native-link&amp;resource=CAP04140\" title=\"Biblioteca Digital de Peças Teatrais (BDTeatro)\" target=\"_blank\">Biblioteca Digital de Peças Teatrais (BDTeatro)</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Outras Fontes&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00216?func=find-db-info&amp;doc_num=000007623\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00217?func=find-db-info&amp;doc_num=000007623','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Biblioteca Digital de Peças Teatrais (BDTeatro)\" title=\"Informação: Biblioteca Digital de Peças Teatrais (BDTeatro)\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00218?func=find-db-add-res&amp;resource=CAP04140&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP04140\",\"000000000\",\"myImageCAP04140\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00219\");return false;'><img name=\"myImageCAP04140\" id=\"myImageCAP04140\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Biblioteca Digital de Peças Teatrais (BDTeatro)\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Biblioteca Digital de Peças Teatrais (BDTeatro)\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-body -->  \
    <tr>  \
      <td style=\"width:10px\" align=\"left\">&nbsp;&nbsp;</td>  \
      <td style=\"width:500px\" align=\"left\"><span id=\"nomedabase\" name=\"nomedabase\"><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00220?func=native-link&amp;resource=CAP02454\" title=\"Biblioteca Digital de Teses e Dissertações : BDTD\" target=\"_blank\">Biblioteca Digital de Teses e Dissertações : BDTD</a> <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-icon-brasil.gif\" alt=\"Icon: brasil\" border=\"0\"></span>&nbsp;</td>  \
      <td style=\"width:220px\" align=\"left\">Teses e Dissertações&nbsp;</td>  \
      <td nowrap=\"nowrap\" align=\"left\">  \
          <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00221?func=find-db-info&amp;doc_num=000004775\" onclick=\"javascript:winPop(event, 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00222?func=find-db-info&amp;doc_num=000004775','yes', 435,545); return false;\">  \
  \
        <img src=\"MetaLib%C2%AE%20-%20Buscar%20Base_files/v-info.png\" alt=\"Informação: Biblioteca Digital de Teses e Dissertações : BDTD\" title=\"Informação: Biblioteca Digital de Teses e Dissertações : BDTD\" style=\"margin-bottom:0px;\"></a>  \
         <!-- <A HREF= 'https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00223?func=find-db-add-res&amp;resource=CAP02454&amp;z122_key=000000000&amp;function-in=www_v_find_db_0' onClick= 'javascript:addToz122(\"CAP02454\",\"000000000\",\"myImageCAP02454\",\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00224\");return false;'><img name=\"myImageCAP02454\" id=\"myImageCAP02454\" src='https://buscador.example.com/INOVO/icon_por/v-add_favorite.png' alt=\"Adicionar às Minhas Pastas\"title=\"Adicionar ao Meu Espaço > Minhas Bases: Biblioteca Digital de Teses e Dissertações : BDTD\" alt=\"Adicionar ao Meu Espaço > Minhas Bases: Biblioteca Digital de Teses e Dissertações : BDTD\"></A> -->  \
<!-- END_SECTION --><!-- body-table-body -->  \
<!-- START_SECTION --><!-- body-table-tail-1 -->  \
      </td>  \
    </tr>  \
<!-- END_SECTION --><!-- body-table-tail-1 -->  \
<!-- START_SECTION --><!-- body-table-tail -->  \
  </tbody></table>  \
  <br>  \
</div> <!-- record_list -->  \
 \
<!-- START INCLUDE FILE NAME:/exlibris/metalib/m4_1/inovo/www_v_por/find-db-3-nav-toolbar --> \
  \
<!--  \
<div class=\"record_list_header\">  \
  <div class=\"containerBottom\">  \
        <div class=\"alignLeft\">  \
                <div class=\"record_list_header_left\">  \
                        <strong>19th - Biblioteca</strong>  \
                </div>  \
                <div class=\"record_list_header_right\">  \
                        Ir para <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00225?func=meta-1&amp;init_type=Y\" title=\"Busca Integrada\" class=\"metaSearch\">Busca Integrada</a>  \
                </div>  \
        </div>  \
        <div class=\"alignRight\">  \
                <span class=\"no_wrap\"><strong><span class=\"fade\">&lt;Anterior</span> <a href='https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00226?func=find-db-3-next&amp;set-entry=000001' title='Próxima Página'>Próximo&gt;</a></strong></span>  \
                </div>  \
  </div>  \
</div>  \
-->  \
  \
<div name=\"basePag\" style=\"float: left; margin-top: -20px; display: block;\">  \
        <script>  \
                baseAtual();  \
        </script>1 - 30 de 256 Base(s)  \
</div>  \
  \
<script>  \
        if(document.getElementsByName('basePag').length == 2) {  \
                document.getElementsByName('basePag')[1].style.display = 'block';  \
        }  \
</script>  \
  \
<div style=\"float:right; margin-top:-20px;\">  \
        <span class=\"no_wrap\"><strong><script>pagNavigationFirst();</script>&lt;&lt; <span class=\"fade\">&lt;Anterior</span> | <a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00227?func=find-db-3-next&amp;set-entry=000001\" title=\"Próxima Página\">Próximo&gt;</a> <script>pagNavigationLast();</script><a href=\"https://buscador.example.com/V/74GJ9QTYYHRUIMKPEC9K63KVPU7R6SKGQV19QK3V8LYTN7X42T-00065?func=find-db-3-next&amp;set-entry=0000211\">&gt;&gt;</a></strong></span>  \
        <br>  \
        <script>  \
                pagNavigation();  \
        </script>Página: 1 de 9  \
</div>  \
 \
<!-- END OF INCLUDE FILE:/exlibris/metalib/m4_1/inovo/www_v_por/find-db-3-nav-toolbar --> \
  \
<!-- END_SECTION --><!-- body-table-tail -->  \
<!-- FILE NAME: /exlibris/metalib/m4_1/inovo/www_v_por/find-db-2-tail --> \
</div>  \
<div id=\"tailwrap\">  \
 \
<!-- START INCLUDE FILE NAME:/exlibris/metalib/m4_1/inovo/www_v_por/tail-include --> \
  \
<div id=\"copyrights\">  \
    <!-- tail-include  -->  \
</div>  \
<!--end popup menu-->  \
  \
<script>  \
var NB = document.getElementsByName(\"nomedabase\");  \
for (var k = 0; k < NB.length; k++) {  \
if (NB[k].innerHTML.match(/¨/)) {  \
 NB[k].innerHTML = NB[k].innerHTML.replace(/¨/gi, \"\");  \
 }  \
}  \
</script>  \
 \
<!-- END OF INCLUDE FILE:/exlibris/metalib/m4_1/inovo/www_v_por/tail-include --> \
  \
</div>  \
  \
  \
 </div></body></html>", ""},
    {NULL, 0, 0, NULL, NULL}
};

double delta(struct timeval *now, struct timeval *old) {
    double ret = 0;
    ret = now->tv_sec - old->tv_sec;
    ret += ((now->tv_usec - old->tv_usec) / 1000000.0);
    return ret;
}

int main(void) {
    PyObject *module = NULL;
    PyObject *controller = NULL;
    pyin_start("/home/rlucca/teste/src/helpers");

    module = pyin_get_module("ecap_logic_reply");
    controller = pyin_create_controller(module, ".squid1.example.com",
                                        "../helpers/dbs");
    struct pyin_adapt_text var = {
        module, NULL,
        NULL, 0,
        NULL, 0
    };

    for (ptr=cases, hhh=headers; ptr->uri != NULL; ptr++) {
        struct timeval begin;
        struct timeval end;
        gettimeofday(&begin, NULL);
        printf("\n\n\n\turi %s\n", ptr->uri);
        //if (!strcmp(ptr->uri, "medicinanet")) { printf("terminacao forcada conforme solicitado\n"); break; }
        printf("%d %s\n", pyin_adapt_begin(&var, controller, ptr->uri, ptr->request_flag), "begin");
        //printf("%p tem que ser alocado!\n", var.state);
        while (ptr->n_header > 0) {
            if (hhh->name) {
                struct pyin_adapt_text var2 = {
                    module, var.state,
                    hhh->value, strlen(hhh->value),
                    NULL, 0
                };
                
                int ret = pyin_adapt_header(&var2, hhh->name);
                /*if (ret > 0 && var2.ab && strcmp(var2.ab, hhh->expected))
                    printf("ret modificador e esperado nao bateu: '%s' == '%s'\n", var2.ab, hhh->expected);
                else if (ret > 0 && hhh->expected[0] == '\0' && var2.ab)
                    printf("ret modificador para exclusao nao obdecido: '%s' %s'\n", var2.ab, hhh->name);*/
                
                free(var2.ab); var2.ab = NULL;
                hhh++;
                var.state = var2.state; // keep the same state that the last one if changed
            }
            ptr->n_header--;
        }
        struct pyin_adapt_text var3 = {
            module, var.state,
            ptr->vb, strlen(ptr->vb),
            NULL, 0
        };
        printf("%d %s\n", pyin_adapt_body(&var3), "erroBody");
        /*if (!var3.ab)
            printf("corpo nulo\n");
        else if (strcmp(var3.ab, ptr->ab))
            if (var3.ab[0] != '\0')
                printf("corpo esperado nao bateu |%s| == |%s|\n", var3.ab, ptr->ab);
            else
                printf("corpo esperado foi postergado pro adapt_end\n");*/
        free(var3.ab); var3.ab = NULL;
        printf("%d adapt end called\n", pyin_adapt_end(&var3));
        /*if (!var3.ab)
            printf("corpo end nulo\n");
        else if (strcmp(var3.ab, ptr->ab))
            printf("corpo end esperado nao bateu |%s| == |%s|\n", var3.ab, ptr->ab);
        else printf("corpo end ok\n");*/
        assert(!var3.state); \
        free(var3.state); // inofensivo pq DEVE estar NULL. remove um warning
        gettimeofday(&end, NULL);
        printf("deltatime_s %f\n", delta(&end, &begin));
    }

    printf("callin finish\n");
    pyin_finish(&module, &controller);
    return 0;
}
