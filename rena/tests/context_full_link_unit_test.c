
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

#include "context_full_link.h"


CHEAT_DECLARE(
/* define group copied from context_full_link { */
#define STATE_BAR       1 // found /
#define STATE_ALLOWED   2 // allowed to process
#define STATE_NONE      3 // not allowed to process
#define STATE_BAR_S     4 // found \\ previously an / go back to BAR or NONE
#define STATE_ALLOWED_S 5 // found \\ inside allowed zone go back to ALLOWED
#define STATE_NONE_S    6 // found \\ inside none zone go back to NONE or BAR
/* } */
const char *states_s[] = {
    "INIT", "1BAR", "ALLO", "NONE" };
	struct context2 {
		context_t c;
		int stage;
	};
	struct database_object {
		struct context2 *context;
	} instance;

	const char test1[] = "Allow ://!\r\nCheck www.example.com or https://www.example.org, okay?";
	int results1[] = {
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_BAR,
        STATE_ALLOWED, STATE_ALLOWED, STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_BAR,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED,
        STATE_ALLOWED, STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE
	};
	int evals1[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 0, 0, 0, 0, 0
	};
	const char test2[] = "Allow :/\\/!\r\nCheck \\/\\www\\.example.com or https:\\/\\/www\\.example.org,\\n okay?";
	int results2[] = {
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_BAR,
        STATE_BAR_S,   STATE_ALLOWED, STATE_ALLOWED, STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE_S,
        STATE_BAR,     STATE_BAR_S,   STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE_S,  STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE_S,  STATE_BAR,     STATE_BAR_S,   STATE_ALLOWED,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED_S,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED,
        STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED, STATE_ALLOWED,
        STATE_ALLOWED, STATE_ALLOWED_S,  STATE_NONE, STATE_NONE,
        STATE_NONE,    STATE_NONE,    STATE_NONE,    STATE_NONE,
        STATE_NONE,
	};
	int evals2[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	};
			
    // malloc e free nao posso trocar!
)

CHEAT_SET_UP(
	memset(&instance, 0, sizeof(instance));
)

CHEAT_TEST(context_set_full_link_parser_nulled,

    context_set_full_link_parser(NULL);
)

CHEAT_TEST(context_set_full_link_parser_instance,

    context_set_full_link_parser(&instance);

    cheat_assert(instance.context != NULL);
    cheat_assert(instance.context->stage == 0);
)

CHEAT_TEST(context_set_full_link_parser_check_stages,

    context_set_full_link_parser(&instance);

    int eflag = -1;
    int len = sizeof(results1) / sizeof(results1[0]);
    for (int K=0; test1[K] != '\0'; K++)
    {
        (void) instance.context->c.parser_fnc(&instance, test1[K]);
        
        if (K >= len || instance.context->stage != results1[K])
        {
            if (K < len)
                fprintf(stderr, "%d - stage wrong [%d] != [%d]\n",
                        K, instance.context->stage, results1[K]);
            if (eflag < 0)
                eflag = K;
        }
    }

    if (eflag >= 0)
        fprintf(stderr, "found error on [%s]\n", test1 + eflag);

    cheat_assert(eflag == -1);
)

CHEAT_TEST(context_set_full_link_parser_check_evaluation,

    context_set_full_link_parser(&instance);

    int eflag = -1;
    int len = sizeof(evals1) / sizeof(evals1[0]);
    for (int K=0; test1[K] != '\0'; K++)
    {
        int res = instance.context->c.parser_fnc(&instance, test1[K]);
        
        if (K >= len || res != evals1[K])
        {
            if (K < len)
                fprintf(stderr, "%d - evaluation wrong [%d] != [%d]\n",
                        K, res, evals1[K]);
            if (eflag < 0)
                eflag = K;
        }
    }

    if (eflag >= 0)
        fprintf(stderr, "found error on [%s]\n", test1 + eflag);

    cheat_assert(eflag == -1);
)

CHEAT_TEST(context_set_full_link_parser_check_stages_safe,

    context_set_full_link_parser(&instance);

    int eflag = -1;
    int len = sizeof(results2) / sizeof(results2[0]);
    for (int K=0; test2[K] != '\0'; K++)
    {
        (void) instance.context->c.parser_fnc(&instance, test2[K]);
        
        if (K >= len || instance.context->stage != results2[K])
        {
            if (K < len)
                fprintf(stderr, "%d - stage wrong [%d] != [%d]\n",
                        K, instance.context->stage, results2[K]);
            if (eflag < 0)
                eflag = K;
        }
    }

    if (eflag >= 0)
        fprintf(stderr, "found error on [%s]\n", test2 + eflag);

    cheat_assert(eflag == -1);
)

CHEAT_TEST(context_set_full_link_parser_check_evaluation_safe,

    context_set_full_link_parser(&instance);

    int eflag = -1;
    int len = sizeof(evals2) / sizeof(evals2[0]);
    for (int K=0; test2[K] != '\0'; K++)
    {
        int res = instance.context->c.parser_fnc(&instance, test2[K]);
        
        if (K >= len || res != evals2[K])
        {
            if (K < len)
                fprintf(stderr, "%d - evaluation wrong [%d] != [%d]\n",
                        K, res, evals2[K]);
            if (eflag < 0)
                eflag = K;
        }
    }

    if (eflag >= 0)
        fprintf(stderr, "found error on [%s]\n", test2 + eflag);

    cheat_assert(eflag == -1);
)
