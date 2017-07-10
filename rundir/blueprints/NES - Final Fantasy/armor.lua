
armor_pieceCount        = 40

--Base stats have 4 bytes per armor piece
--  byte 0 = evade penalty
--  byte 1 = absorb bonus
--  byte 2 = elemental resistance (bitflags)
--  byte 3 = spell cast
armor_baseStatOffset    = 0x30150


importArmor = function(file)
    file:seek( "set", armor_baseStateOffset )
    
    for i=0, armor_pieceCount-1 do
        local id = string.format("%d", i)
        local evade, absorb, element, spell = file:read(4):byte(1,4)

        lsh.set( "armor.evade."..id,    evade )
        lsh.set( "armor.absorb."..id,   absorb )
        lsh.set( "armor.element."..id,  element )
        lsh.set( "armor.spell."..id,    spell )
    end
end


exportArmor = function(file)
    file:seek( "set", armor_baseStateOffset )
    
    for i=0, armor_pieceCount-1 do
        id = string.format("%d", i)
        evade =     lsh.get( "armor.evade."..id )
        absorb =    lsh.get( "armor.absorb."..id )
        element =   lsh.get( "armor.element."..id )
        spell =     lsh.get( "armor.spell."..id )
        
        file:write( string.char( evade, absorb, element, spell ) )
    end
    

end