#pragma once

#include "CmPrerequisites.h"
#include "CmFontDesc.h"
#include "CmInt2.h"

namespace CamelotFramework
{
	class CM_EXPORT TextUtility
	{
	private:
		class TextWord
		{
		public:
			TextWord(bool spacer);

			void addChar(const CHAR_DESC& desc);
			void addSpace(UINT32 spaceWidth);

			void removeLastChar();

			UINT32 getWidth() const { return mWidth; }
			UINT32 getHeight() const { return mHeight; }
			bool isSpacer() const { return mSpacer; }

			const vector<CHAR_DESC>::type& getChars() const { return mChars; }

		private:
			vector<CHAR_DESC>::type mChars;
			UINT32 mWidth;
			UINT32 mHeight;
			bool mSpacer;
			UINT32 mSpaceWidth;

			void calculateWidth();
		};

	public:
		class CM_EXPORT TextLine
		{
		public:
			TextLine(UINT32 baselineOffset, UINT32 lineHeight, UINT32 spaceWidth);
			~TextLine();

			UINT32 getWidth() const { return mWidth; }
			UINT32 getHeight() const { return mHeight; }

			/**
			 * @brief	Returns an offset used to separate two lines.
			 */
			UINT32 getYOffset() const { return mLineHeight; }

			/**
			 * @brief	Gets a number quads used by all characters for every page used by this text line.
			 *
			 * @note	One page generally corresponds to one bitmap from which the characters are read from.
			 * 			
			 *			One character is represented with a single quad. Some pages might be empty.
			 */
			vector<UINT32>::type getNumQuadsPerPage() const;

			/**
			 * @brief	Fills the vertex/uv/index buffers for the specified page, with all the character data
			 * 			needed for rendering.
			 *
			 * @param	page			The page.
			 * @param [out]	vertices	Pre-allocated array where character vertices will be written.
			 * @param [out]	uvs			Pre-allocated array where character uv coordinates will be written.
			 * @param [out]	indexes 	Pre-allocated array where character indices will be written.
			 * @param	offset			Offsets the location at which the method writes to the buffers.
			 * 							Counted as number of quads.
			 * @param	size			Total number of quads that can fit into the specified buffers.
			 *
			 * @return	Number of quads that were written.
			 */
			UINT32 fillBuffer(UINT32 page, Vector2* vertices, Vector2* uvs, UINT32* indexes, UINT32 offset, UINT32 size) const;

		private:
			friend class TextUtility;

			UINT32 mWidth;
			UINT32 mHeight;
			UINT32 mBaselineOffset;
			UINT32 mLineHeight;
			UINT32 mSpaceWidth;
			vector<TextWord>::type mWords;
			TextWord* mLastWord;

			void add(const CHAR_DESC& charDesc);
			void addSpace();
			void addWord(const TextWord& word);

			TextWord removeLastWord();

			void calculateBounds();
		};

		class CM_EXPORT TextData
		{
		public:
			~TextData();

			const vector<TextLine>::type& getLines() const { return mLines; }
			const vector<HTexture>::type& getTexturePages() const { return mTexturePages; }
			const vector<UINT32>::type& getNumQuadsPerPage() const  { return mQuadsPerPage; }
			UINT32 getWidth() const;
			UINT32 getHeight() const;

		private:
			friend class TextUtility;

			vector<UINT32>::type mQuadsPerPage;
			vector<TextLine>::type mLines;
			vector<HTexture>::type mTexturePages;
		};

		static std::shared_ptr<TextUtility::TextData> getTextData(const WString& text, const HFont& font, UINT32 fontSize, UINT32 width = 0, UINT32 height = 0, bool wordWrap = false);
	};
}