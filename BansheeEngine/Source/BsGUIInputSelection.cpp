//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsGUIInputSelection.h"
#include "BsSpriteTexture.h"
#include "BsImageSprite.h"
#include "BsGUIElement.h"
#include "BsGUIManager.h"

namespace BansheeEngine
{
	GUIInputSelection::GUIInputSelection()
		:mSelectionStart(0), mSelectionEnd(0), mSelectionAnchor(0), mSelectionDragAnchor(0)
	{ }

	GUIInputSelection::~GUIInputSelection()
	{
		for(auto& sprite : mSprites)
			bs_delete<PoolAlloc>(sprite);
	}

	void GUIInputSelection::updateSprite()
	{
		mSelectionRects = getSelectionRects();

		INT32 diff = (INT32)(mSprites.size() - mSelectionRects.size());

		if(diff > 0)
		{
			for(UINT32 i = (UINT32)mSelectionRects.size(); i < (UINT32)mSprites.size(); i++)
				bs_delete(mSprites[i]);

			mSprites.erase(mSprites.begin() + mSelectionRects.size(), mSprites.end());
		}
		else if(diff < 0)
		{
			for(INT32 i = diff; i < 0; i++)
			{
				ImageSprite* newSprite = bs_new<ImageSprite>();
				mSprites.push_back(newSprite);
			}
		}

		UINT32 idx = 0;
		for(auto& sprite : mSprites)
		{
			IMAGE_SPRITE_DESC desc;
			desc.width = mSelectionRects[idx].width;
			desc.height = mSelectionRects[idx].height;
			desc.texture = GUIManager::instance().getTextSelectionTexture().getInternalPtr();

			sprite->update(desc);
			idx++;
		}
	}

	Vector2I GUIInputSelection::getSelectionSpriteOffset(UINT32 spriteIdx) const
	{
		return Vector2I(mSelectionRects[spriteIdx].x, mSelectionRects[spriteIdx].y) + getTextOffset();
	}

	RectI GUIInputSelection::getSelectionSpriteClipRect(UINT32 spriteIdx, const RectI& parentClipRect) const
	{
		Vector2I selectionOffset(mSelectionRects[spriteIdx].x, mSelectionRects[spriteIdx].y);
		Vector2I clipOffset = selectionOffset + mElement->_getTextInputOffset();

		RectI clipRect(-clipOffset.x, -clipOffset.y, mTextDesc.width, mTextDesc.height);

		RectI localParentCliprect = parentClipRect;

		// Move parent rect to our space
		localParentCliprect.x += mElement->_getTextInputOffset().x + clipRect.x;
		localParentCliprect.y += mElement->_getTextInputOffset().y + clipRect.y;

		// Clip our rectangle so its not larger then the parent
		clipRect.clip(localParentCliprect);

		// Increase clip size by 1, so we can fit the caret in case it is fully at the end of the text
		clipRect.width += 1;

		return clipRect;
	}

	Vector<RectI> GUIInputSelection::getSelectionRects() const
	{
		Vector<RectI> selectionRects;

		if(mSelectionStart == mSelectionEnd)
			return selectionRects;

		UINT32 startLine = getLineForChar(mSelectionStart);

		UINT32 endLine = startLine;
		if(mSelectionEnd > 0)
			endLine = getLineForChar(mSelectionEnd - 1, true);

		{
			const GUIInputLineDesc& lineDesc = getLineDesc(startLine);

			UINT32 startCharIdx = mSelectionStart;

			UINT32 endCharIdx = mSelectionEnd - 1;
			if(startLine != endLine)
			{
				endCharIdx = lineDesc.getEndChar(false);
				if(endCharIdx > 0)
					endCharIdx = endCharIdx - 1;
			}

			if(!isNewlineChar(startCharIdx) && !isNewlineChar(endCharIdx))
			{
				RectI startChar = getLocalCharRect(startCharIdx);
				RectI endChar = getLocalCharRect(endCharIdx);

				RectI selectionRect;
				selectionRect.x = startChar.x;
				selectionRect.y = lineDesc.getLineYStart();
				selectionRect.height = lineDesc.getLineHeight();
				selectionRect.width = (endChar.x + endChar.width) - startChar.x;

				selectionRects.push_back(selectionRect);
			}
		}

		for(UINT32 i = startLine + 1; i < endLine; i++)
		{
			const GUIInputLineDesc& lineDesc = getLineDesc(i);
			if(lineDesc.getStartChar() == lineDesc.getEndChar() || isNewlineChar(lineDesc.getStartChar()))
				continue;

			UINT32 endCharIdx = lineDesc.getEndChar(false);
			if(endCharIdx > 0)
				endCharIdx = endCharIdx - 1;

			RectI startChar = getLocalCharRect(lineDesc.getStartChar());
			RectI endChar = getLocalCharRect(endCharIdx);

			RectI selectionRect;
			selectionRect.x = startChar.x;
			selectionRect.y = lineDesc.getLineYStart();
			selectionRect.height = lineDesc.getLineHeight();
			selectionRect.width = (endChar.x + endChar.width) - startChar.x;

			selectionRects.push_back(selectionRect);
		}

		if(startLine != endLine)
		{
			const GUIInputLineDesc& lineDesc = getLineDesc(endLine);

			if(lineDesc.getStartChar() != lineDesc.getEndChar() && !isNewlineChar(lineDesc.getStartChar()))
			{
				UINT32 endCharIdx = mSelectionEnd - 1;

				if(!isNewlineChar(endCharIdx))
				{
					RectI startChar = getLocalCharRect(lineDesc.getStartChar());
					RectI endChar = getLocalCharRect(endCharIdx);

					RectI selectionRect;
					selectionRect.x = startChar.x;
					selectionRect.y = lineDesc.getLineYStart();
					selectionRect.height = lineDesc.getLineHeight();
					selectionRect.width = (endChar.x + endChar.width) - startChar.x;

					selectionRects.push_back(selectionRect);
				}
			}
		}

		return selectionRects;
	}

	void GUIInputSelection::showSelection(UINT32 anchorCaretPos)
	{
		UINT32 charIdx = getCharIdxAtInputIdx(anchorCaretPos);

		mSelectionStart = charIdx;
		mSelectionEnd = charIdx;
		mSelectionAnchor = charIdx;
	}

	void GUIInputSelection::clearSelection()
	{
		for(auto& sprite : mSprites)
			bs_delete(sprite);

		mSprites.clear();
	}

	void GUIInputSelection::selectionDragStart(UINT32 caretPos)
	{
		clearSelection();

		showSelection(caretPos); 
		mSelectionDragAnchor = caretPos;
	}

	void GUIInputSelection::selectionDragUpdate(UINT32 caretPos)
	{
		if(caretPos < mSelectionDragAnchor)
		{
			mSelectionStart = getCharIdxAtInputIdx(caretPos);
			mSelectionEnd = getCharIdxAtInputIdx(mSelectionDragAnchor);

			mSelectionAnchor = mSelectionStart;
		}

		if(caretPos > mSelectionDragAnchor)
		{
			mSelectionStart = getCharIdxAtInputIdx(mSelectionDragAnchor);
			mSelectionEnd = getCharIdxAtInputIdx(caretPos);

			mSelectionAnchor = mSelectionEnd;
		}

		if(caretPos == mSelectionDragAnchor)
		{
			mSelectionStart = mSelectionAnchor;
			mSelectionEnd = mSelectionAnchor;
		}
	}

	void GUIInputSelection::selectionDragEnd()
	{
		if(isSelectionEmpty())
			clearSelection();
	}

	void GUIInputSelection::moveSelectionToCaret(UINT32 caretPos)
	{
		UINT32 charIdx = getCharIdxAtInputIdx(caretPos);

		if(charIdx > mSelectionAnchor)
		{
			mSelectionStart = mSelectionAnchor;
			mSelectionEnd = charIdx;
		}
		else
		{
			mSelectionStart = charIdx;
			mSelectionEnd = mSelectionAnchor;
		}

		if(mSelectionStart == mSelectionEnd)
			clearSelection();
	}

	void GUIInputSelection::selectAll()
	{
		mSelectionStart = 0;
		mSelectionEnd = (UINT32)mTextDesc.text.size();
	}

	bool GUIInputSelection::isSelectionEmpty() const
	{
		return mSelectionStart == mSelectionEnd;
	}
}