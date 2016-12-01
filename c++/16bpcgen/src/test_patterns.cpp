#include <sys/stat.h>
#include <unistd.h>
#include "Image.hpp"
#include "PatternGenerators.hpp"

int main(void)
{
	mkdir("./img", 0755);
	const column_t width  = 1920;
	const row_t    height = 1080;

	Image(width, height) << Luster(white) << TypeWriter(__FILE__, black) >> "./img/sourcecode.png";

	Image image(width, height);
	image << ColorBar()               >> "./img/colorbar.png";
	image << Luster(white)            >> "./img/white100.png";
	image << Luster(red)              >> "./img/red100.png";
	image << Luster(green)            >> "./img/green100.png";
	image << Luster(blue)             >> "./img/blue100.png";
	image << Luster(white/2)          >> "./img/white50.png";
	image << Luster(red/2)            >> "./img/red50.png";
	image << Luster(green/2)          >> "./img/green50.png";
	image << Luster(blue/2)           >> "./img/blue50.png";
	image << Checker()                >> "./img/checker1.png";
	image << Checker(true)            >> "./img/checker2.png";
	image << StairStepH()             >> "./img/stairstepH1.png";
	image << StairStepH(1, 20, false) >> "./img/stairstepH2.png";
	image << StairStepH(1, 20, true)  >> "./img/stairstepH3.png";
	image << StairStepV()             >> "./img/stairstepV1.png";
	image << StairStepV(1, 20, false) >> "./img/stairstepV2.png";
	image << StairStepV(1, 20, true)  >> "./img/stairstepV3.png";
	image << Ramp()                   >> "./img/ramp.png";
	image << Luster(black)
			<< CrossHatch(width/10, height/10)   >> "./img/crosshatch.png";
	image << Luster(black)
			<< Character(" !\"#$%&'()*+,-./\n"
						"0123456789:;<=>?@\nABCDEFGHIJKLMNO\nPQRSTUVWXYZ[\\]^_`\n"
						"abcdefghijklmno\npqrstuvwxyz{|}~", red, 10) >> "./img/character.png";
#if 201103L <= __cplusplus
	image << WhiteNoise() >> "./img/whitenoise.png";
#endif

	return 0;
}
