XMLText* XMLDocument::NewText( const char* str )
{
    XMLText* text = CreateUnlinkedNode<XMLText>( _textPool );
    text->SetValue( str );
    return text;
}