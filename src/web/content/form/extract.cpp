#include "bacs/archive/web/content/form/extract.hpp"

namespace bacs{namespace archive{namespace web{namespace content{namespace form
{
    extract::extract()
    {
        ids.name("ids");
        ids.message(cppcms::locale::translate("Problem ids"));
        ids.message(cppcms::locale::translate("Extract"));
        add(config);
        add(ids);
        add(submit);
    }
}}}}}