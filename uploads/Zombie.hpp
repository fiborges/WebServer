/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Zombie.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei < brolivei@student.42porto.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/06 13:23:04 by brolivei          #+#    #+#             */
/*   Updated: 2024/01/02 14:49:24 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include <iostream>

/*
		Primeira parte do exercício é então criar a classe Zombie.

		Diz as instruções que:
			- Tem uma string como atributo privado.

			- Tem um metodo (função membro) tal que: void	announce(void);

		Os Zombie anunciam-se da seguinte forma:

			<name>: BraiiiinnnzzzZ...

		Após isso, temos a implementação de duas outras funções:

			São elas:

				- Zombie	*newZombie(std::string name);

					Esta função, cria um novo zombie, da-lhe um nome, e retorna um
				ponteiro para o local de memória deste novo zombie (objeto), de maneira
				que ele pode ser usado fora da memória da função.


				- void		randomChump(std::string name);

					Esta função cria o zombie, e faz com que ele se anuncie.

			Porque declarar estas funções com estáticas???

				Estas funções são feitas para ser associadas à classe, e não apenas
			aos objetos da classe.

				É o caso da função newZombie.Ela é feita para criar um novo Zombie e retornar
			um ponteiro para ele. Portanto, faz sentido, que ela possa ser chamada sem que
			primeiro tenhamos que criar um objeto da classe "Zombie".

				De forma semelhante, a função randomChump é feita para criar também um
			novo zombie, e fazer com que ele se anuncie. Então, faz todo o sentido também
			poder chama-la diretamente sem que para isso, tenhamos que cria um objeto zombie.
*/

class	Zombie
{
	private:
		std::string	name;
	public:
		Zombie(std::string input_name);
		~Zombie();
		void	announce(void);
};

Zombie*	newZombie(std::string name);
void	randomChump(std::string name);

#endif
