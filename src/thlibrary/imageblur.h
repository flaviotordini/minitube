/*
 *   This is an adaptation of Jani Huhtanen Exponential blur code.
 *
 *   Copyright 2007 Jani Huhtanen <jani.huhtanen@tut.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _KDE_PLASMA_BLUR_H_
#define _KDE_PLASMA_BLUR_H_

class QImage;

class ImageBlur {
	public:
		ImageBlur (QImage *image, int aprec, int zprec);
		~ImageBlur();

	public:
		void expblur (int radius);

	public:
		static void expblur (QImage *image, int aprec, int zprec, int radius);

	private:
		void blurcol (int col, int alpha);
		void blurrow (int line, int alpha);

		void blurinner (unsigned char *bptr,
						int &zR, int &zG, int &zB, int &zA,
						int alpha);

	private:
		QImage *m_image;
		int m_aprec;
		int m_zprec;
};

#endif /* !_KDE_PLASMA_BLUR_H_ */

