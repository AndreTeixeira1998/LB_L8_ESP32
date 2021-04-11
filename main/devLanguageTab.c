#include "devLanguageTab.h"

#include "devDataManage.h"

#include "string.h"

/*
	针对 lvFont_yahei_20.c
	win10选择微软雅黑 常规 10号字体， win7选择微软雅黑 常规 13号字体
*/

LV_FONT_DECLARE(lv_font_dejavu_15)
LV_FONT_DECLARE(lv_font_dejavu_bold_15)
LV_FONT_DECLARE(lv_font_dejavu_20)
LV_FONT_DECLARE(lv_font_consola_19)
LV_FONT_DECLARE(lv_font_consola_16)
LV_FONT_DECLARE(lv_font_consola_17)
LV_FONT_DECLARE(lv_font_dejavu_30)
LV_FONT_DECLARE(lv_font_arabic_15)
LV_FONT_DECLARE(lv_font_arabic_17)
LV_FONT_DECLARE(lv_font_arabic_24)

static enumDevLanguageIdentify systemLanguageCountryIdentify = languageCyId_English;

const struct stt_tagLabelStringTab{

	enumLabelStr tlsFlg;
	char *nTagLabel_tab[DEV_LANGUAGE_COUNTRY_NUM_MAX];
	
}tabLabelString[] = {

	{tls_sysInit, 				{"System\ninitializing...", "\n...ماظنلا ةئيهت", 					"Inicialização\ndo sistema"}		},
	{tls_sysRestart,			{"Sys restart", 			"ليغشت ةداعإ", 							"  Reiniciar"}						},
	{tls_wifiScanning,			{"   wifi\n   scanning...", "ياف ياو\n...يئوضلا حسملا", 			"  Análise\nWi-Fi..."}				},
	{tls_wifiConnecting,		{"Wifi\nconnecting...", 	"\n...ياف ياو لاصتا", 					"\nConexão Wi-Fi..."}				},
	{tls_wifiConnected,			{"\nWifi connected", 		"   wifi\nلاصتالا مامتإ", 				"Conexão Wi-Fi\ncompleta"}			},
	{tls_wifiReconnected,		{"Wifi\nreconnecting...", 	"wifi\n...لاصتالا ةداعإ", 				"\nReconexão Wi-Fi..."}				},
	{tls_HAserverChanged,		{"\nHA server changed", 	"\nHA تريغت دق مداخلا", 				"HA Alterações\ndo servidor"}		},
	{tls_serverChanged,			{"\nServer changed", 		"\nتريغت دق مداخلا", 					"Alterações\ndo servidor"}			},
	{tls_illmiCfgChanged,		{"Illumination\nconfig changed", "ةنيز حابصم\nنيوكتلا نم ءاهتنالا",   "Luzes decorativas\ndefinidas"}	},
	{tls_paramChanging,			{"\nParam changing", 		"\n...ةملعملا رييغت", 					"Mudar OS\nparâmetros..."}			},
	{tls_devListGenerate,		{"List\ngenerateing...", 	"\n...ىوتحملا ءاشنإ", 					"Geração de\nconteúdos..."}			},
	
	{tls_Menu,					{"Menu",					"وينم", 								"Cardapio",}						},
	{tls_Timer,					{"Timer",					"تقوم", 								"Cronograma",}						},
	{tls_Delayer,				{"Delayer",					"ريخأت", 								"Ddemora",}							},
	{tls_LinkageCfg,			{"Linkage\nConfig",			"وبموك", 								"Interativo",}						},
	{tls_WifiCfg,				{"Wifi\nConfig",			"ةكبش\nنيوكت", 							"Rede",}							},
	{tls_Setting,				{"Setting",					"وينم", 								"Configuração",}					},
	{tls_SysInfo,				{"System\nInfo",			"ماظنلا\nتامولعم", 						"Informação",}						},

	{tls_confirm,				{"Confirm",					"ديكأت",								"Confirmar",}						},
	{tls_save,					{"Save",					"ظفح",									"Preservação",}						},
	{tls_cancel,				{"Cancel",					"ءاغلإ",								"Cancelar",}						},
	
	{tls_hour,					{"hour",					"ةعاس",									"Hora",}							},
	{tls_minute,				{"minute",					"قئاقد",								"Minuto",}							},
	{tls_min,					{"min",						"قئاقد",								"M",}								},
	{tls_second,				{"second",					"ةيناث",								"Segundo",}							},
	{tls_sec,					{"sec",						"ةيناث",								"S",}								},

	{tls_mTimer_timerSetting,	{"Timer Setting",			"ينمزلا لودجلا دادعإ",					"Cronograma",}						},
	{tls_mTimer_noRepeat,		{"No repeat",				"رركتت ال",								"Sem repetição",}					},
	{tls_mTimer_switchSetting,	{"Switch Setting:",			":ليدبتلا عضو",							"Status de resposta:",}				},
	{tls_mTimer_repeatSetting,	{"Repeat Setting:",			":راركتلا ةعومجم",						"Definição do Ciclo:",}				},
	{tls_mTimer_timeSetting,	{"Time Setting:", 			":تقولا ديدحت",							"Tempo:",}							},
	{tls_mTimer_repeatCycle,	{"Repeat cycle",			"ةرودلا راركت",							"Período de repetição",}			},
	{tls_mTimer_Monday,			{" Monday",					"نينثالا موي",							"Segunda-feira",}					},
	{tls_mTimer_Tuesday,		{" Tuesday",				"ءاثالثلا",								"Terça-feira",}						},
	{tls_mTimer_Wednesday,		{" Wednesday",				"ءاعبرالا",								"Quarta-feira",}					},
	{tls_mTimer_Thursday,		{" Thursday",				"سيمخلا",								"Quinta-feira",}					},
	{tls_mTimer_Friday,			{" Friday",					"ةعمجلا",								"Sexta-feira",}						},
	{tls_mTimer_Saturday,		{" Saturday",				"تبسلا",								"Sábado",}							},
	{tls_mTimer_Sunday,			{" Sunday",					"دحألا",								"Domingo",}							},

	{tls_mDelayer_delaySetting,	{"Delay setting",			"تقؤم نييعت",							"Definição de atraso",}				},
	{tls_mDelayer_Delay,		{"Delay",					"ريخأت",								"Atraso",}							},
	{tls_mDelayer_GreenMode,	{"Greenmode",				"رضخألا عضولا",							"Modo Verde",}						},
	{tls_mDelayer_timeSet,		{"Time set:",				":تقولا ديدحت",							"Tempo:",}							},
	{tls_mDelayer_trigSet,		{"Trig set:",				":دادعإلا ةلاح",						"Status de resposta:",}				},
	{tls_mDelayer_enable,		{"Enable:",					":حمس",									"Habilitar:"}						},
	{tls_mDelayer_statusNormal,	{"Status Normal:",			":ةيعيبط ةلاح",							"Comportamento normal:"}			},
	{tls_mDelayer_actionObj,	{"Action Object:",			":تكجبوا",								"Alvo Da execução:"}				},
	
	{tls_mLinkage_linkageCfg,	{"Linkage Config",			"تالصولا تادادعإ",						"Interativo"}						},
	{tls_mLinkage_unsupported,	{"Function Unsupported",	"معدت ال ةيلاحلا تازيملا", 				"Esta função não é suportada\nNeste momento.",}},
	{tls_mLinkage_voltageOption,{"Voltage Option",			"دهجلا تارايخ",							"Opções de tensão",}				},
	{tls_mLinkage_tempOption,	{"Temperature Option",		"ةرارحلا ةجرد تارايخ",					"Opções de temperatura",}			},
	{tls_mLinkage_alarmTemp,	{"Alarm Temperature:",		"؄؄؄؄؄؄؄؄:هيبنتلا ةرارح ةجرد",			"Temperatura de alarme:",}			},
	{tls_mLinkage_devList,		{"Devlist Option",			"ةزهجألا ةمئاق تارايخ",					"Opções de lista de dispositivos",}	},
	{tls_mLinkage_volOpen,		{"Voltage Open:",			"؄؄؄؄؄؄؄؄؄؄:دهجلا ىلع هرودب",			"Ligue a tensão:",}					},
	{tls_mLinkage_volClose,		{"Voltage Close:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄:دهجلا فاقيإ",			"Desligue a tensão:",}				},
	{tls_mLinkage_volAlarmEn,	{"Alarm Enable:",			"؄؄؄؄؄؄؄؄؄؄؄:هيبنتلا حمسي",				"Alarme permitido:",}				},

	{tls_mSetting_deviceDefine,	{"Device Define",			"ماظنلا تادادعإ",						"Configuração"}						},
	{tls_mSetting_deviceType,	{"Device Type:",			":زاهجلا عون",							"Tipo de equipamento:"}				},
	{tls_mSetting_deviceType_1,	{"Device Type",				"زاهجلا عون",							"Tipo de equipamento"}				},
	{tls_mSetting_systemLanguage,{"System Language:", 		":ماظنلا ةغل",							"Língua do sistema:"}				},
	{tls_mSetting_systemLanguage_1,{"System Language", 		"ماظنلا ةغل",							"Língua do sistema"}				},
	{tls_mSetting_switchType,	{"Switch Type",				":ليدبتلا عون",							"Tipo de comutador:"}				},
	{tls_mSetting_stayTheStatus,{"Stay the Status:",		":حالة الذاكرة",						"Estado Da memória:"}				},
	{tls_mSetting_bGroundPicSet,{"Background Pic Set:",		":ةيفلخلا ةطيرخلا دادعإ", 				"Definir Imagem de Fundo:",}		},
	{tls_mSetting_decoractiveSet,{"Decorative Lamp Set:",	":تادادعإ ةفرخزلا ءاوضأ",  				"Configurar luzes decorativas:",}	},
	{tls_mSetting_illminationSet,{"Illumination Set",		"تادادعإ ءاوضأ",						"Luzes decorativas",}				},
	{tls_mSetting_automatic,	{"Automatic",				"ايئاقلت",  							"Automático",}						},
	{tls_mSetting_preview,		{"Preview:",				":ةنياعم",								"Antevisão",}						},
	{tls_mSetting_setAllSwitches,{"Set all switches",		"تادعملا عيمج غالبإ",					"Aplicar tudo L8",}					},
	{tls_mSetting_breathingEffect,{"Breathing Effect",		"سفنتلا ءوض ريثأت",						"Efeito de respiração",}			},
	{tls_mSetting_theme,		{"Theme:",					":بولسأ",								"Tema:",}							},
	{tls_mSetting_uiKindIndex,		{"Interface Type:",					":نوع الواجهة",								"Tipo de Interface:",}							},
	{tls_mSetting_theme1,		{"","","",}},
	{tls_mSetting_theme2,		{"","","",}},
	{tls_mSetting_theme3,		{"","","",}},
	{tls_mSetting_factoryReset,	{"Factory Reset:",			":عنصملا تادادعإ دعأ",					"Reiniciar sistema:",}				},
	{tls_mSetting_devieManage,	{"Device Manager:",			":ةرادإلا تادعم",						"Gestão do dispositivo:",}			},
	{tls_mSetting_homassistantSupport,{"Homeassistant Support:",":Homeassistantةقفاوتم",			"Homeassistant Compatíveis:",}		},
	{tls_mSetting_screenBrightness,{"Screen Brightness:",	":ةشاشلا عوطس",							"Brilho de tela:",}					},
	{tls_mSetting_tempParamDisp, {"Temperature unit:",":درجة الحرارة وحدة",		"Unidade de temperatura:",}	},
	{tls_mSetting_tempDispCels, {"Celsius",":درجة مئوية",		"Celsius",}	},
	{tls_mSetting_tempDispFahr, {"Fahrenheit",":فهرنهايت",		"Fahrenheit",}	},
	{tls_mSetting_elecParamDisp,{"Electric Parameter Display:",":ةيئابرهكلا تارتمارابلا ضرع",		"Exibir parâmetros elétricos:",}	},
	{tls_mSetting_elecDispPower,{" Power",					":ةرباع ةوق",							"Potência",}						},
	{tls_mSetting_elecDispSum,	{" Electric Sum",			"ءابرهكلا كالهتسا يلامجإ", 				"Consumo de Energia",}				},
	{tls_mSetting_elecSumReset,	{"Electric Sum Reset:",		"ءابرهكلا تالجس غارفإ",					"Consumo de Energia LiMPO:",}		},
	{tls_mSetting_temperatureCalibretion,{"Temperature Calibration:",":ةرارحلا ةرياعم",				"Calibração Da temperatura:",}		},
	{tls_mSetting_screenLightTime,{"Screen Light Time:",	":تقولا تادادعإ ةشاشلا ءيضت",			"Hora Da tela:",}					},
	{tls_mSetting_screenDispAngle,{"Screen Display Angle:",	":ةيوازلا تادادعإ ضرع", 				"Ângulo de exibição:",}				},
	{tls_mSetting_vertical,		{" Vertical",				"يدومع",								" Vertical",}						},
	{tls_mSetting_horizontal,	{" Horizontal",				"يقفأ",									" Horizontal",}						},
	{tls_mSetting_upgrade,		{"Firmware Upgrade:",		":ماظنلا ةيقرت",						"Actualização do sistema:",}		},
	{tls_mSetting_epidemicReport,{"COVID19 Report:",		":عضولا COVID19",						"COVID19 Apresentação:",}			},
	{tls_mSetting_epidemicZone,	{"Location",				"ميلاقأ", 								"Região",}							},
	
	{tls_mWifiCfg_wifiCfg,		{"Wifi Config",				"نيوكت WiFi",							"Operação de rede",}				},
	{tls_mWifiCfg_noCfg,		{"","","",}},
	{tls_mWifiCfg_wifiSSID,		{"SSID:",					":SSID",								"SSID:",}							},
	{tls_mWifiCfg_wifiPassword,	{"Password:",				":رورملاملك",							"Senha:",}							},
	{tls_mWifiCfg_wifiStatus,	{"Wifi Status:",			":ةلود Wifi",							"Estado de Wi-Fi:",}				},
	{tls_mWifiCfg_methodSelect,	{"Method Select:",			":قرطلا رتخا",							"Selecção do modo:",}				},
	{tls_mWifiCfg_QrCode,		{"Qr Code",					"ريفش",									"Código QR",}						},
	{tls_mWifiCfg_reset,		{"Reset",					"طبض ةداعإ",							"Reiniciar",}						},
	
	{tls_mSysInfo_sysInfo,		{"System Info",				"ماظنلا تامولعم",						"Informação",}						},
	{tls_mSysInfog_sysTime,		{"System Time:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:تقولا",			"Tempo do sistema:"}				},
	{tls_mSysInfo_sVersion,		{"Version:",				"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:ةخسنلا",			"Edição:",}							},
	{tls_mSysInfo_ePower,		{"Device Power:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:ةرباع ةوق",		"Potência do equipamento:",}		},
	{tls_mSysInfo_eSum,			{"Electricity Consumption:","؄؄؄؄؄؄؄؄؄؄؄؄؄؄ :ةقاطلا كالهتسا", 		"Consumo de Energia do equipamento:",}},
	{tls_mSysInfo_freeHeap,		{"Free heap:",				"؄؄؄؄؄؄؄؄؄؄؄  :ةيقبتملا ةحاسملا",		"Memória restante:",}				},
	{tls_mSysInfo_freeInternalHeapMini, {"Free heap(interMini):","؄؄؄؄؄؄؄؄؄؄؄  :ةيقبتملا ةحاسملا",	"Memória restante:",}				},
	{tls_mSysInfo_topicBssid,	{"Topic bssid:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄:لاصتالا عيضاوم",		"Temas de comunicação:",}			},
	{tls_mSysInfo_DeviceMac,	{"Device mac:",				"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:زاهجلا مقر ",		"Endereço do dispositivo:",}		},
	{tls_mSysInfo_parentBssid,	{"Parent bssid:",			"؄؄؄؄؄؄؄؄؄؄؄:ايلعلا ةزهجألا مقر ",		"Endereço do dispositivo Pai:",}	},
	{tls_mSysInfo_parentRSSI,	{"Parent RSSI:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:ةراشإلا ةوق",		"Intensidade do sinal:",}			},
	{tls_mSysInfo_meshNodeNum,	{"Mesh node num:",			"؄؄؄؄؄؄؄؄:تادعملل يلامجإلا ددعلا ",        "Número de nós de rede:",}},
	{tls_mSysInfo_meshLayer,	{"Mesh layer:",				"؄؄؄؄؄؄؄؄؄؄؄؄؄؄:ةكبشلا ىوتسم ",			"Hierarquia Da rede:",}},
	{tls_mSysInfo_deviceMutualGroup,{"Device mutual group:","؄؄؄؄؄؄:لدابتملا مكحتلا تامولعم ", 		"Informações de controlo mútuo:",}	},
	{tls_mSysInfo_deviceTemperature,{"Device temperature:",	"؄؄؄؄؄؄؄؄؄؄؄؄؄:زاهجلا ةرارح ةجرد ", 	"Temperatura do equipamento:",}		},
	{tls_mSysInfo_machineTime,	{"Mechine time:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:زاهجلا تقو ",		"Hora Da máquina:",}				},
	{tls_mSysInfo_parentSSID,	{"Parent SSID:",			"؄؄؄؄؄؄؄؄؄؄؄؄:هيجوتلا زاهج مسا ",		"Nome Da rota:",}					},
	{tls_mSysInfo_routerName,	{"Router name:",			"؄؄؄؄؄؄؄؄؄؄؄؄:هيجوتلا زاهج مسا ",		"Nome Da rota:",}					},
	{tls_mSysInfo_routerPassword,{"Router password:",		"؄؄؄؄؄؄؄:هيجوتلا زاهج رورم ةملك ", 		"Senha de rota:",}					},
	{tls_mSysInfo_timeZone,		{"Time zone:",				"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:تقولا ةقطنم",		"Zona temporal:",}					},
	{tls_mSysInfo_versionNum,	{"Version num:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:رادصإلا مقر",		"Número de versão:",}				},
	{tls_mSysInfo_hardwareType,	{"Hardware type:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:ةزهجألا عون",		"Tipo de hardware:",}				},
	{tls_mSysInfo_networkInfo,	{"Network info:",			"؄؄؄؄؄؄؄؄؄؄؄؄؄؄:ةكبشلا تامولعم",		"Informação Da rede:",}				},
	{tls_mSysInfo_sunScale,		{"Sun scale:", 				"؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄؄:Sun scale",			"Escala de sol:",}					},
	{tls_mSysInfo_sunScaleLocationInfo, {"Sun scale locInfo:", "؄؄؄؄؄؄؄؄؄؄؄:Sun scale locInfo", 	"Sun scale locInfo:",}				},
	
	{tls_null,					{NULL}},
};	

const struct stt_tagDialogWordsTab{

	enumDialogWordsStr dialogTipsFlg;
	char *nTagWords_tab[DEV_LANGUAGE_COUNTRY_NUM_MAX];
	
}tabDialogTipsString[] = {

	{	dialogTips_restart,
		{"Factory reset operation will restart the device automatically, Are you sure to continue?",			
		 "رارمتسالاديرت كنأ نم دكأتم تنأ له ، زاهجلا ليغشت ةداعإ متيس عنصملا تادادعإ ؟ةداعتسا",				
		 "O reset do sistema Precisa reiniciar para fazer efeito. Você TEM certeza de continuar?",}
	},

	{	dialogTips_nwkReset,
		{"The current operation will delete the WiFi network and restart the device automatically, Are you sure to continue?",			
		 "رارمتسالا ديرت كنأ نم دكأتم تنأ له ، ةكبشلا نيوكت ةداعإ ؟فوس ةيلاحلا ةيلمعلا",			
		 "A operação atual irá limpar a informação Da rede e reiniciar o dispositivo. Você TEM certeza de continuar?",}
	},

	{	dialogTips_restart2Effect,
		{"The current operation needs to be restarted to take effect, Are you sure to continue?",			
		 "رارمتسالا ديرتكنأ نم دكأتم تنأ له ، ةلاعف حبصت نأ لبق ليغشت ةداعإ ىلإ جاتحت ؟ةيلاحلا ةيلمعلا",				
		 "A operação atual Precisa reiniciar o dispositivo para fazer efeito. Tem certeza que deseja continuar?",}
	},
		
	{	dialogTips_reboot2Effect,
		{"The current operation needs to be restarted to take effect, Are you sure to continue?",			
		 "رارمتسالا ديرتكنأ نم دكأتم تنأ له ، ةلاعف حبصت نأ لبق ليغشت ةداعإ ىلإ جاتحت ؟ةيلاحلا ةيلمعلا",				
		 "A operação atual Precisa reiniciar o dispositivo para fazer efeito. Tem certeza que deseja continuar?",}
	},

	{	dialogTips_upgrade,	
		{"the system will be updated, are you sure to continue?",			
		 "رارمتسالا ديرت كنأ نم دكأتم تنأ له ، ماظنلا؟ ةيقرت متيس",				
		 "O sistema será atualizado. Você TEM certeza de continuar?",}
	},

	{	dialogTips_elecClr,	
		{"Electricity statistics will be cleared, Are you sure to continue?",			
		 "رارمتسالا ديرت كنأ نم دكأتم تنأ له ، ةقاطلا تانايب؟ حسم متيس",				
		 "O consumo de Energia do dispositivo será limpo. Deseja continuar?",}
	},

	{	dialogTips_opetSuccess,
		{"Opreation is successful.",
		 "عملية ناجحة",
		 "Operação bem sucedida.",}
	},

	{dialogTips_null, {NULL}},
};

const struct stt_tagDialogSelTabTab{

	enumDialogSelTab dialogSelTabFlg;
	const char *mbox_btnm_textTab[DEV_LANGUAGE_COUNTRY_NUM_MAX + 1][16];
	
}tabDialogSelTab[] = {

	{	dialogSelTab_ok,
		{	{"OK", ""},
			{"ديج هنا", ""},
			{"bem", ""},
		}
	},

	{	dialogSelTab_yesNo,
		{	{"Yes", "No", ""},
			{"معن", "لا", ""},
			{"Sim", "Não", ""},
		}
	},

	{	dialogSelTab_ObjSelMesh,
		{	{"Self", "All", "Cancel", ""},
			{"Self", "All", "Cancel", ""},
			{"Self", "All", "Cancel", ""},
		}
	},

	{dialogSelTab_null, {{NULL}}},
};

uint8_t systemLanguage_strTagIndexGet(enumDevLanguageIdentify countryId){

	uint8_t res = 0;

	switch(countryId){

		case languageCyId_English:
			res = 0;
			break;

		case languageCyId_Arabic:
			res = 1;
			break;

		case languageCyId_Portuguese:
			res = 2;
			break;

		default:break;
	}

	return res;
}

const char *labelStrGet_bySysLanguage(enumLabelStr strTag){

	const char *strRes = NULL;
	uint8_t loop = 0;

	do{
		if(strTag == tabLabelString[loop].tlsFlg){
			strRes = tabLabelString[loop].nTagLabel_tab[systemLanguage_strTagIndexGet(systemLanguageCountryIdentify)];
			break;
		}
		loop ++;
	}while(tls_null != tabLabelString[loop].tlsFlg);

	return strRes;
}

char *dialogWordsStrGet_bySysLanguage(enumDialogWordsStr strTag){

	const char *strRes = NULL;
	char *strWords = (char *)os_zalloc(sizeof(char) * 256);
	uint8_t loop = 0;

	do{
		if(strTag == tabDialogTipsString[loop].dialogTipsFlg){
			strRes = tabDialogTipsString[loop].nTagWords_tab[systemLanguage_strTagIndexGet(systemLanguageCountryIdentify)];
			break;
		}
		loop ++;
	}while(dialogTips_null != tabDialogTipsString[loop].dialogTipsFlg);

	if(NULL != strRes){

		switch(systemLanguageCountryIdentify){
			
//			case languageCyId_Arabic:{
//				uint8_t strLen = strlen(strRes),
//						opIndex = 0;
//				for(loop = 0; loop < (strLen / 2); loop ++){
//					opIndex = strLen - (2 * (loop + 1));
//					memcpy(&strWords[loop * 2], &strRes[opIndex], 2);
//				}
//			}break;

			default:
				strcpy(strWords, strRes);
				break;
		}
	}

	return strWords;
}

const char **dialogSelTabGet_bySysLanguage(enumDialogSelTab strTag){

	const char **strRes = NULL;
	uint8_t loop = 0;

	do{
		if(strTag == tabDialogSelTab[loop].dialogSelTabFlg){
			strRes = tabDialogSelTab[loop].mbox_btnm_textTab[systemLanguage_strTagIndexGet(systemLanguageCountryIdentify)];
			break;
		}
		loop ++;
	}while(dialogSelTab_null != tabDialogSelTab[loop].dialogSelTabFlg);

	return strRes;
}

const lv_font_t *fontLabelGet_bySysLanguage(uint8_t f_style){

	lv_font_t *res = NULL;

	switch(systemLanguageCountryIdentify){

		case languageCyId_English:{
			switch(f_style){
				case 0:
					res = &lv_font_dejavu_15;
//					res = &lv_font_dejavu_bold_15;
					break;
				case 1:
					res = &lv_font_dejavu_15;
//					res = &lv_font_dejavu_bold_15;
					break;
				case 2:
					res = &lv_font_consola_16;
					break;
				case 3:
//					res = &lv_font_consola_17;
					res = &lv_font_consola_16;
					break;
				case 4:
					res = &lv_font_consola_19;
					break;
				case 5:
					res = &lv_font_dejavu_20;
					break;
				case 6:
					res = &lv_font_dejavu_30;
					break;
				default:
					res = &lv_font_consola_16;
					break;
			}	
		}break;
		case languageCyId_Arabic:{
			switch(f_style){
				case 0:
					res = &lv_font_arabic_15;
					break;
				case 1:
					res = &lv_font_arabic_17;
					break;
				case 6:
					res = &lv_font_arabic_24;
					break;
				default:
					res = &lv_font_arabic_17;
					break;
			}
		}break;
		case languageCyId_Portuguese:{
			switch(f_style){
				case 0:
					res = &lv_font_dejavu_15;
					break;
				case 1:
					res = &lv_font_dejavu_15;
					break;
				case 2:
					res = &lv_font_consola_16;
					break;
				case 3:
					res = &lv_font_consola_17;
					break;
				case 4:
					res = &lv_font_consola_17;
					break;
				case 5:
					res = &lv_font_consola_17;
					break;
				case 6:
					res = &lv_font_dejavu_30;
					break;
				default:
					res = &lv_font_consola_17;
					break;
			}
		}break;

		default:break;
	}

	return res;
}

void devLanguageCountry_identifySet(enumDevLanguageIdentify id,  bool nvsRecord_IF){

	stt_sysLanguageNvsAttr dpOpr = {0};

	systemLanguageCountryIdentify = id;
	dpOpr.sysLanguage = systemLanguageCountryIdentify;
	if(nvsRecord_IF){

		devSystemInfoLocalRecord_save(saveObj_systemLanguage, &dpOpr);
	}
}

enumDevLanguageIdentify devLanguageCountry_identifyGet(void){

	return systemLanguageCountryIdentify;
}



























